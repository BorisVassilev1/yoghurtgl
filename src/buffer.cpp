#include <buffer.h>

ygl::Buffer::Buffer(ygl::Buffer &&other) {
	this->buffer = other.buffer;
	this->target = other.target;
	this->size	 = other.size;
	other.buffer = 0;
	other.size	 = 0;
}

ygl::Buffer &ygl::Buffer::operator=(ygl::Buffer &&other) {
	if (this != &other) {
		glDeleteBuffers(1, &buffer);
		this->buffer = other.buffer;
		this->target = other.target;
		this->size	 = other.size;
		other.buffer = 0;
		other.size	 = 0;
	}
	return *this;
}

void ygl::Buffer::bind(GLenum target) {
	assert(buffer != 0 && "please initialize the buffer");
	this->target = target;
	glBindBuffer(target, buffer);
	bound = true;
}

void ygl::Buffer::unbind() { glBindBuffer(target, 0); }

void ygl::Buffer::set(void *data, GLsizeiptr size, GLsizeiptr offset) {
	Bind b(this);
	glBufferSubData(target, offset, size, data);
}

void ygl::Buffer::get(void *data, GLsizeiptr size, GLsizeiptr offset) {
	Bind b(this);
	glGetBufferSubData(target, offset, size, data);
}

ygl::Bind::Bind(ygl::Buffer &buffer) : buffer(&buffer) {
	wasBound = buffer.isBound();
	buffer.bind(buffer.getTarget());
}

ygl::Bind::Bind(ygl::Buffer *buffer) : Bind(*buffer) {}

ygl::Bind::~Bind() {
	if (buffer && !wasBound) buffer->unbind();
}

ygl::Bind::Bind(ygl::Bind &&other) {
	this->buffer   = other.buffer;
	this->wasBound = other.wasBound;
	other.buffer   = nullptr;
	other.wasBound = false;
}

ygl::Bind &ygl::Bind::operator=(ygl::Bind &&other) {
	if (this != &other) {
		this->buffer   = other.buffer;
		this->wasBound = other.wasBound;
		other.buffer   = nullptr;
		other.wasBound = false;
	}
	return *this;
}
