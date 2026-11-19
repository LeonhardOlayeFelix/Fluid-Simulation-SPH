#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BoundingBox
{
private:
	glm::vec2 min{};
	glm::vec2 max{};

public:
	BoundingBox() = default;
	BoundingBox(glm::vec2 xminymin, glm::vec2 xmaxymax) : min(xminymin), max(xmaxymax) {};

	inline float xmin() const { return min.x; }
	inline float ymin() const { return min.y; }
	inline float xmax() const { return max.x; }
	inline float ymax() const { return max.y; }
	inline float width() const { return max.x - min.x; }
	inline float height() const { return max.y - min.y; }
	inline float xcentre() const { return min.x + width() / 2; }
	inline float ycentre() const { return min.y + height() / 2; }
	inline glm::vec2 centre() const { return { xcentre(), ycentre() }; }
	inline glm::vec2 size() const { return { width(), height() }; }


};

