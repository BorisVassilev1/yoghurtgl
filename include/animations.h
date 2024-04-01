#pragma once
#include <vector>

#include <yoghurtgl.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/anim.h>
#include <assimp/Importer.hpp>
#include <assimp_glm_helpers.h>

#include <window.h>
#include <shader.h>
#include <mesh.h>

namespace ygl {

struct KeyPosition {
	glm::vec3 position;
	float	  timeStamp;
};

struct KeyRotation {
	glm::quat orientation;
	float	  timeStamp;
};

struct KeyScale {
	glm::vec3 scale;
	float	  timeStamp;
};

enum AnimationBehaviour {
	Loop,
	Stop
};

class Bone {
   private:
	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotations;
	std::vector<KeyScale>	 m_Scales;
	int						 m_NumPositions;
	int						 m_NumRotations;
	int						 m_NumScalings;

	size_t currentPositionIndex = 0;
	size_t currentRotationIndex = 0;
	size_t currentScaleIndex	= 0;
	float  currentTime			= 0;
	AnimationBehaviour behaviour;

	glm::mat4	m_LocalTransform;
	glm::vec3	m_LocalTranslation;
	glm::quat	m_LocalRotation;
	glm::vec3	m_LocalScale;
	std::string m_Name;
	int			m_ID;
	float		m_Duration;

   public:
	/*reads keyframes from aiNodeAnim*/
	Bone(const std::string& name, int ID, const aiNodeAnim* channel, float duration, AnimationBehaviour behaviour = Loop);

	/*interpolates  b/w positions,rotations & scaling keys based on the curren time of
	the animation and prepares the local transformation matrix by combining all keys
	tranformations*/
	void Update(float animationTime);

	glm::mat4&	GetLocalTransform() { return m_LocalTransform; }
	glm::vec3&	GetLocalTranslation() { return m_LocalTranslation; }
	glm::quat&	GetLocalRotation() { return m_LocalRotation; }
	glm::vec3&	GetLocalScale() { return m_LocalScale; }
	std::string GetBoneName() const { return m_Name; }
	int			GetBoneID() { return m_ID; }

	/* Gets the current index on mKeyPositions to interpolate to based on
	the current animation time*/
	uint GetPositionIndex(float animationTime);

	/* Gets the current index on mKeyRotations to interpolate to based on the
	current animation time*/
	uint GetRotationIndex(float animationTime);

	/* Gets the current index on mKeyScalings to interpolate to based on the
	current animation time */
	uint GetScaleIndex(float animationTime);

   private:
	/* Gets normalized value for Lerp & Slerp*/
	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

	/*figures out which position keys to interpolate b/w and performs the interpolation
	and returns the translation matrix*/
	glm::vec3 InterpolatePosition(float animationTime);

	/*figures out which rotations keys to interpolate b/w and performs the interpolation
	and returns the rotation matrix*/
	glm::quat InterpolateRotation(float animationTime);

	/*figures out which scaling keys to interpolate b/w and performs the interpolation
	and returns the scale matrix*/
	glm::vec3 InterpolateScaling(float animationTime);
};

struct AssimpNodeData {
	glm::mat4					transformation;
	std::string					name;
	int							childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation {
   public:
	Animation() = default;

	Animation(const aiScene* scene, uint index, AnimationBehaviour behaviour = Loop) {
		assert(scene && scene->mRootNode);
		auto animation = scene->mAnimations[index];
		if (animation->mNumMeshChannels != 0) { dbLog(ygl::LOG_WARNING, "loading an animation with mesh channels"); }
		if (animation->mNumChannels == 0) { dbLog(ygl::LOG_WARNING, "loading an animation with no node animation"); }
		m_Duration		 = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;
		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, behaviour);
	}

	Bone* FindBone(const std::string& name) {
		auto iter = m_BoneInfoMap.find(name);
		if (iter != m_BoneInfoMap.end()) { return &m_Bones[iter->second.id]; }
		return nullptr;
	}

	inline float GetTicksPerSecond() { return m_TicksPerSecond; }

	inline float GetDuration() { return m_Duration; }

	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

	inline const std::unordered_map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

	inline uint GetBonesCount() { return m_Bones.size(); }

   private:
	void ReadMissingBones(const aiAnimation* animation, AnimationBehaviour behaviour) {
		int size = animation->mNumChannels;

		int boneCount = 0;

		// reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++) {
			auto		channel	 = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;
			fixMixamoBoneName(boneName);
			if (!m_BoneInfoMap.contains(boneName)) {
				BoneInfo newBoneInfo;
				newBoneInfo.id			= boneCount;
				m_BoneInfoMap[boneName] = newBoneInfo;
				++boneCount;
			}
			auto id = m_BoneInfoMap.find(boneName)->second.id;
			m_Bones.push_back(Bone(boneName, id, channel, m_Duration, behaviour));
		}
	}

	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
		assert(src);

		dest.name = src->mName.data;
		fixMixamoBoneName(dest.name);

		dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount	= src->mNumChildren;

		for (uint i = 0; i < src->mNumChildren; i++) {
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	float									  m_Duration;
	int										  m_TicksPerSecond;
	std::vector<Bone>						  m_Bones;
	AssimpNodeData							  m_RootNode;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
};

class Animator {
   public:
	Animator(AnimatedMesh* mesh, Animation* currentAnimation) {
		m_CurrentTimeCurrent = 0.0;
		m_CurrentTimeBlended = 0.0;
		m_CurrentAnimation	 = currentAnimation;
		this->mesh			 = mesh;

		m_FinalBoneMatrices.reserve(200);
		for (uint i = 0; i < 200; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));

		glGenBuffers(1, &matricesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, matricesBuffer);
		glBufferData(GL_ARRAY_BUFFER, GetFinalBoneMatrices().size() * 4 * 16, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void UpdateAnimation(float dt) {
		m_DeltaTime = dt;
		if (m_CurrentAnimation) {
			m_CurrentTimeCurrent += m_CurrentAnimation->GetTicksPerSecond() * dt;
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
		UpdateBoneBuffer();
	}

	void UpdateAnimationBlended(float dt, float factor) {
		m_DeltaTime = dt;
		if (m_CurrentAnimation) {
			m_CurrentTimeCurrent += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTimeBlended += m_BlendedAnimation->GetTicksPerSecond() * dt;
			CalculateBoneTransformBlended(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f), factor);
		}
		UpdateBoneBuffer();
	}

	void UpdateBoneBuffer() {
		glBindBuffer(GL_ARRAY_BUFFER, matricesBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_FinalBoneMatrices.size() * 64, m_FinalBoneMatrices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		Shader::setUBO(matricesBuffer, 3);
	}

	void PlayAnimation(Animation* pAnimation) {
		m_CurrentAnimation = pAnimation;
		m_CurrentTimeCurrent	   = 0.0f;
	}

	void setBlendAnimation(Animation* animation) {
		m_BlendedAnimation	 = animation;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
		const std::string& nodeName = node->name;
		glm::mat4		   nodeTransform;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone) {
			Bone->Update(m_CurrentTimeCurrent);
			nodeTransform = Bone->GetLocalTransform();
		} else {
			nodeTransform = node->transformation;
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto& boneInfoMap = mesh->getBoneInfoMap();
		auto  boneInfo	  = boneInfoMap.find(nodeName);
		if (boneInfo != boneInfoMap.end()) {
			int		  index			   = boneInfo->second.id;
			glm::mat4 offset		   = boneInfo->second.offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	void CalculateBoneTransformBlended(const AssimpNodeData* node, glm::mat4 parentTransform, float factor) {
		assert(m_BlendedAnimation != nullptr);
		const std::string& nodeName = node->name;
		glm::mat4		   nodeTransform;

		Bone* Bone1 = m_CurrentAnimation->FindBone(nodeName);
		Bone* Bone2 = m_BlendedAnimation->FindBone(nodeName);

		if (Bone1 && Bone2) {
			Bone1->Update(m_CurrentTimeCurrent);
			Bone2->Update(m_CurrentTimeBlended);
			glm::vec3 translation = glm::mix(Bone1->GetLocalTranslation(), Bone2->GetLocalTranslation(), factor);
			glm::quat rotation	  = glm::slerp(Bone1->GetLocalRotation(), Bone2->GetLocalRotation(), factor);
			rotation			  = glm::normalize(rotation);
			glm::vec3 scale		  = glm::mix(Bone1->GetLocalScale(), Bone2->GetLocalScale(), factor);

			glm::mat4 t	  = glm::translate(glm::mat4(1), translation);
			glm::mat4 r	  = glm::toMat4(rotation);
			glm::mat4 s	  = glm::scale(glm::mat4(1), scale);
			nodeTransform = t * r * s;
		} else {
			nodeTransform = node->transformation;
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto& boneInfoMap = mesh->getBoneInfoMap();
		auto  boneInfo	  = boneInfoMap.find(nodeName);
		if (boneInfo != boneInfoMap.end()) {
			int		  index			   = boneInfo->second.id;
			glm::mat4 offset		   = boneInfo->second.offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransformBlended(&node->children[i], globalTransformation, factor);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices() { return m_FinalBoneMatrices; }

   private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation*			   m_CurrentAnimation;
	Animation*			   m_BlendedAnimation;
	float				   m_CurrentTimeCurrent;
	float				   m_CurrentTimeBlended;
	float				   m_DeltaTime;
	AnimatedMesh*		   mesh;
	uint				   matricesBuffer;
};

class AnimationFSM {
	Animator*				animator;
	std::vector<Animation*> animations;
	float					blendFactor		= 0.;
	uint					currAnimation	= 0;
	uint					nextAnimation	= 0;
	float					transitionSpeed = 3;

   public:
	AnimationFSM(Animator* animator, Animation* animation) : animator(animator) {
		addAnimation(animation);
		animator->PlayAnimation(animations[currAnimation]);
		animator->setBlendAnimation(animations[currAnimation]);
	}

	void addAnimation(Animation* animation) { animations.push_back(animation); }

	void transition(uint newAnimation) {
		currAnimation = nextAnimation;
		nextAnimation = newAnimation;
		animator->PlayAnimation(animations[nextAnimation]);
		animator->setBlendAnimation(animations[currAnimation]);
		blendFactor = 1.0;
	}

	void update(float dt) {
		blendFactor = glm::clamp<float>(blendFactor - transitionSpeed * dt, 0, 1);

		animator->UpdateAnimationBlended(dt, blendFactor);
	}

	float getBlendFactor() {return blendFactor;}
};

};	   // namespace ygl
