#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct BoundingBox
{
private:
	glm::vec2 m_Min{};
	glm::vec2 m_Max{};

public:
	BoundingBox() = default;
	BoundingBox(glm::vec2 xminymin, glm::vec2 xmaxymax) : m_Min(xminymin), m_Max(xmaxymax) {};

	inline void SetMin(glm::vec2 xminymin) { m_Min = xminymin; }
	inline void SetMax(glm::vec2 xmaxymax) { m_Max = xmaxymax; }

	inline float xmin() const { return m_Min.x; }
	inline float ymin() const { return m_Min.y; }
	inline float xmax() const { return m_Max.x; }
	inline float ymax() const { return m_Max.y; }
	inline float width() const { return m_Max.x - m_Min.x; }
	inline float height() const { return m_Max.y - m_Min.y; }
	inline float xcentre() const { return m_Min.x + width() / 2; }
	inline float ycentre() const { return m_Min.y + height() / 2; }
	inline float left() const { return xcentre() - width() / 2; }
	inline float right() const { return xcentre() + width() / 2; }
	inline float down() const { return ycentre() - height() / 2; }
	inline float up() const { return ycentre() + height() / 2; }
	inline BoundingBox getScaledAboutCentre(float d) const 
	{
		glm::vec2 centre = 0.5f * (m_Min + m_Max);
		return BoundingBox(centre + d * (m_Min - centre), centre + d * (m_Max - centre));
	}
	inline BoundingBox getInset(float d) const { return BoundingBox(m_Min + glm::vec2(d), m_Max - glm::vec2(d)); }

	inline glm::vec2 centre() const { return { xcentre(), ycentre() }; }
	inline glm::vec2 size() const { return { width(), height() }; }
	inline glm::vec2 min() const { return m_Min; }
	inline glm::vec2 max() const { return m_Max; }
	inline glm::vec2 ndcScaleFactor() const { return { width() / 2, height() / 2 }; };
	inline glm::vec2 ndcTranslation() const { return { xcentre(), ycentre() }; };

};

