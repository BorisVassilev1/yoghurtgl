#include <animations.h>

ygl::Bone::Bone(const std::string &name, int ID, const aiNodeAnim *channel, float duration, AnimationBehaviour behaviour)
	: behaviour(behaviour), m_LocalTransform(1.0f), m_Name(name), m_ID(ID), m_Duration(duration) {
	m_NumPositions = channel->mNumPositionKeys;

	for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
		aiVector3D	aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float		timeStamp  = channel->mPositionKeys[positionIndex].mTime;
		KeyPosition data;
		data.position  = AssimpGLMHelpers::GetGLMVec(aiPosition);
		data.timeStamp = timeStamp;
		m_Positions.push_back(data);
	}

	m_NumRotations = channel->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float		 timeStamp	   = channel->mRotationKeys[rotationIndex].mTime;
		KeyRotation	 data;
		data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
		data.timeStamp	 = timeStamp;
		m_Rotations.push_back(data);
	}

	m_NumScalings = channel->mNumScalingKeys;
	for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
		aiVector3D scale	 = channel->mScalingKeys[keyIndex].mValue;
		float	   timeStamp = channel->mScalingKeys[keyIndex].mTime;
		KeyScale   data;
		data.scale	   = AssimpGLMHelpers::GetGLMVec(scale);
		data.timeStamp = timeStamp;
		m_Scales.push_back(data);
	}
}

void ygl::Bone::Update(float animationTime) {
	switch (behaviour) {
		case Loop: animationTime = fmod(animationTime, m_Duration); break;
		case Stop: animationTime = glm::min<float>(animationTime, m_Duration - 0.01); break;
		default: assert(false && "invalid animation behaviour");
	}
	glm::vec3 translation = InterpolatePosition(animationTime);
	glm::quat rotation	  = InterpolateRotation(animationTime);
	glm::vec3 scale		  = InterpolateScaling(animationTime);
	glm::mat4 t			  = glm::translate(glm::mat4(1), translation);
	glm::mat4 r			  = glm::toMat4(rotation);
	glm::mat4 s			  = glm::scale(glm::mat4(1), scale);
	m_LocalTransform	  = t * r * s;
	m_LocalTranslation	  = translation;
	m_LocalRotation		  = rotation;
	m_LocalScale		  = scale;
	currentTime			  = animationTime;
}

template <class T>
static int getIndex(float animationTime, std::vector<T> keys, float &currentAnimationTime, size_t &currentIndex) {
	size_t startIndex = 0;
	if (animationTime >= currentAnimationTime) { startIndex = currentIndex; }

	for (size_t index = startIndex; index < keys.size() - 1; ++index) {
		if (animationTime < keys[index + 1].timeStamp) {
			currentIndex = index;
			return index;
		}
	}
	return keys.size() - 1;
}

uint ygl::Bone::GetPositionIndex(float animationTime) {
	return getIndex(animationTime, m_Positions, currentTime, currentPositionIndex);
}

uint ygl::Bone::GetRotationIndex(float animationTime) {
	return getIndex(animationTime, m_Rotations, currentTime, currentRotationIndex);
}

uint ygl::Bone::GetScaleIndex(float animationTime) {
	return getIndex(animationTime, m_Scales, currentTime, currentScaleIndex);
}

float ygl::Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
	float scaleFactor  = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff   = nextTimeStamp - lastTimeStamp;
	scaleFactor		   = midWayLength / framesDiff;
	return scaleFactor;
}

glm::vec3 ygl::Bone::InterpolatePosition(float animationTime) {
	if (1 == m_NumPositions) return m_Positions[0].position;

	uint p0Index = GetPositionIndex(animationTime);
	uint p1Index = p0Index + 1;
	if (p1Index >= m_Positions.size()) --p1Index;
	float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
	glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
	return finalPosition;
}

glm::quat ygl::Bone::InterpolateRotation(float animationTime) {
	if (1 == m_NumRotations) {
		auto rotation = glm::normalize(m_Rotations[0].orientation);
		return rotation;
	}

	uint p0Index = GetRotationIndex(animationTime);
	uint p1Index = p0Index + 1;
	if (p1Index >= m_Rotations.size()) --p1Index;
	float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
	glm::quat finalRotation =
		glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
	finalRotation = glm::normalize(finalRotation);
	return finalRotation;
}

glm::vec3 ygl::Bone::InterpolateScaling(float animationTime) {
	if (1 == m_NumScalings) return m_Scales[0].scale;

	uint p0Index = GetScaleIndex(animationTime);
	uint p1Index = p0Index + 1;
	if (p1Index >= m_Scales.size()) --p1Index;
	float	  scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
	glm::vec3 finalScale  = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
	return finalScale;
}
