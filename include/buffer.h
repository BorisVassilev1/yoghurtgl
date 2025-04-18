#pragma once
#include <yoghurtgl.h>

namespace ygl {
class Buffer {
   public:
	Buffer() = default;
	Buffer(GLenum target, GLsizeiptr size) : target(target), size(size) { glGenBuffers(1, &buffer); }
	~Buffer() { glDeleteBuffers(1, &buffer); }

	Buffer(const Buffer &) = delete;
	Buffer(Buffer &&other);
	Buffer &operator=(const Buffer &) = delete;
	Buffer &operator=(Buffer &&other);

	void bind(GLenum target);
	void unbind();

	void set(void *data, GLsizeiptr size, GLsizeiptr offset = 0);

	GLuint	   getID() const { return buffer; }
	GLenum	   getTarget() const { return target; }
	GLsizeiptr getSize() const { return size; }
	bool	   isBound() const { return bound; }

   protected:
	GLuint	   buffer = 0;
	GLenum	   target = GL_ARRAY_BUFFER;
	GLsizeiptr size	  = 0;
	bool	   bound  = false;
};

class Bind {
	Buffer *buffer	 = nullptr;
	bool	wasBound = false;

   public:
	Bind(Buffer &buffer);
	Bind(Buffer *buffer);
	~Bind();
	Bind(const Bind &) = delete;
	Bind(Bind &&other) ;
	Bind &operator=(const Bind &) = delete;
	Bind &operator=(Bind &&other);
};

class ImmutableBuffer : public Buffer {
   public:
	ImmutableBuffer(GLenum target, GLsizeiptr size, GLbitfield flags) : Buffer(target, size) {
		Bind b(this);
		glBufferStorage(target, size, nullptr, flags);
	}

   private:
};

class MutableBuffer : public Buffer {
   public:
	MutableBuffer() = default;
	MutableBuffer(GLenum target, GLsizeiptr size, GLenum usage) : Buffer(target, size), usage(usage) {
		if(size <= 0) dbLog(ygl::LOG_WARNING, "Buffer size is 0 or less: ", (int64_t)size);
		Bind b(this);
		glBufferData(target, size, nullptr, usage);
	}

	void resize(GLsizeiptr size) {
		if (size == this->size) return;
		Bind b(this);
		glBufferData(target, size, nullptr, usage);
		this->size = size;
	}
	private:
	GLenum usage;
};

};	   // namespace ygl
