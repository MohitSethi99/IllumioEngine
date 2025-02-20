#pragma once

#include <glm/glm.hpp>
#include <IlluminoEngine.h>

namespace IlluminoEngine
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
		
		void OnUpdate(Timestep timestep);

		void SetViewportSize(float width, float height);
		void SetPosition(const glm::vec3 position) { m_Position = position; }
		void SetYaw(float yaw) { m_Yaw = yaw; }
		void SetPitch(float pitch) { m_Pitch = pitch; }

		const glm::vec3& GetPosition() const { return m_Position; }
		const glm::vec3& GetForward() const { return m_Forward; }
		const glm::vec3& GetRight() const { return m_Right; }
		const glm::vec3& GetUp() const { return m_Up; }
		const float GetYaw() const { return m_Yaw; }
		const float GetPitch() const { return m_Pitch; }

		virtual const glm::mat4& GetTransform() const override { return glm::inverse(m_View); }
		virtual const glm::mat4& GetView() const override { return m_View; }
		virtual const glm::mat4& GetProjection() const override { return m_Projection; }

	private:
		float m_Fov = 45.0f;
		float m_AspectRatio = 1.777f;
		float m_NearClip = 0.03f;
		float m_FarClip = 1000.0f;

		glm::vec3 m_Position = glm::vec3(0, 0, -2.5);
		glm::vec3 m_Forward = glm::vec3(0, 0, 1);
		glm::vec3 m_Right = glm::vec3(1, 0, 0);
		glm::vec3 m_Up = glm::vec3(0, 1, 0);

		float m_Yaw = 90.0f;
		float m_Pitch = 0.0f;

		glm::mat4 m_View = glm::mat4(1.0f);
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};
}
