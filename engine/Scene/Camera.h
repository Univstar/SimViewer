#pragma once

#include "Scene/Coordinates.h"

namespace Pivot {
	class Camera {
	public:
		virtual ~Camera() = default;

		virtual void SetAs(Camera const &rhs) = 0;
		virtual glm::mat4 GetTransform(float aspect) = 0;
		virtual glm::vec3 GetPosition() = 0;

		// Screen input handlers
		virtual void Rotate(float dx, float dy) { }
		virtual void Translate(float dx, float dy) { }
		virtual void Scale(float delta) { }

		void Rotate(glm::vec2 const &delta) { Rotate(delta.x, delta.y); }
		void Translate(glm::vec2 const &delta) { Translate(delta.x, delta.y); }
	};

	class OrthoCamera : public Camera {
	public:
		explicit OrthoCamera(glm::vec2 const &center = { 0, 0 }, float height = 1.f) :
			m_Center { center }, m_Height { height } {
		}

		virtual void SetAs(Camera const &rhs) override { *this = *reinterpret_cast<OrthoCamera const *>(&rhs); }

		virtual glm::mat4 GetTransform(float aspect) override {
			float const halfHeight = m_Height * .5f;
			float const halfWidth  = aspect * halfHeight;
			return glm::ortho(m_Center.x - halfWidth, m_Center.x + halfWidth, m_Center.y - halfHeight, m_Center.y + halfHeight);
		}

		virtual glm::vec3 GetPosition() override { return { 0, 0, 1 }; }

		virtual void Translate(float dx, float dy) override {
			m_Center += glm::vec2(-dx, dy) * m_Height;
		}

		virtual void Scale(float delta) override {
			m_Height *= std::exp(-.05f * delta);
		}

		auto GetCenter() const { return m_Center; }
		auto GetHeight() const { return m_Height; }

	private:
		glm::vec2 m_Center;
		float     m_Height;
	};

	class PerspectiveCamera : public Camera {
	public:
		explicit PerspectiveCamera(
			glm::vec3 eye     = { 0, 0, 1 },
			glm::vec3 center  = { 0, 0, 0 },
			float     fovy    = glm::radians(45.f),
			float     zNear   = .1f,
			float     zFar    = 100.f) :
			m_Eye { eye }, m_Center { center },
			m_Fovy { fovy }, m_ZNear { zNear }, m_ZFar { zFar } {
		}

		virtual ~PerspectiveCamera() = default;

		virtual void SetAs(Camera const &rhs) override { *this = *reinterpret_cast<PerspectiveCamera const *>(&rhs); }

		virtual glm::mat4 GetTransform(float aspect) override { return glm::perspective(m_Fovy, aspect, m_ZNear, m_ZFar) * glm::lookAt(m_Eye, m_Center, glm::vec3(0, 1, 0)); }

		virtual glm::vec3 GetPosition() override { return m_Eye; }
	
		auto GetEye()    const { return m_Eye; }
		auto GetCenter() const { return m_Center; }
		auto GetFovy()   const { return m_Fovy; }
		auto GetZNear()  const { return m_ZNear; }
		auto GetZFar()   const { return m_ZFar; }
	
	protected:
		glm::vec3 m_Eye;
		glm::vec3 m_Center;
		float     m_Fovy;
		float     m_ZNear;
		float     m_ZFar;
	};

	class OrbitCamera : public PerspectiveCamera {
	public:
		explicit OrbitCamera(
			glm::vec3 center = { 0, 0, 0 },
			float     rad    = 1,
			float     theta  = glm::radians(45.f),
			float     phi    = glm::radians(270.f),
			float     fovy   = glm::radians(45.f),
			float     zNear  = .1f,
			float     zFar   = 100.f) :
			PerspectiveCamera(
				center + Coordinates::SphericalToCartesian(rad, theta, phi),
				center, fovy, zNear, zFar),
			m_Rad { rad }, m_Theta { theta }, m_Phi { phi } {
		}

		virtual void SetAs(Camera const &rhs) override { *this = *reinterpret_cast<OrbitCamera const *>(&rhs); }

		virtual void Rotate(float dx, float dy) override {
			m_Phi   -= glm::radians(90.f) * dx;
			m_Theta -= glm::radians(90.f) * dy;
			m_Theta = std::clamp(m_Theta, glm::radians(.1f), glm::radians(179.9f));
			m_Phi = std::fmod(m_Phi, glm::radians(360.f));
			if (m_Phi < 0) m_Phi += glm::radians(360.f);
			m_Eye = m_Center + Coordinates::SphericalToCartesian(m_Rad, m_Theta, m_Phi);
		}

		virtual void Translate(float dx, float dy) override {
			glm::vec3 const front = glm::normalize(m_Center - m_Eye);
			glm::vec3 const right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
			glm::vec3 const up    = glm::normalize(glm::cross(right, front));
			m_Center += (up * dy - right * dx) * m_Rad * 2.f * std::tan(m_Fovy * .5f);
			m_Eye = m_Center + Coordinates::SphericalToCartesian(m_Rad, m_Theta, m_Phi);
		}

		virtual void Scale(float delta) override {
			m_Rad *= std::exp(-.05f * delta);
			m_Rad = std::clamp(m_Rad, 1e-4f, 1e+4f);
			m_Eye = m_Center + Coordinates::SphericalToCartesian(m_Rad, m_Theta, m_Phi);
		}

		auto GetRad()   const { return m_Rad; }
		auto GetTheta() const { return m_Theta; }
		auto GetPhi()   const { return m_Phi; }
	
	private:
		float m_Rad;
		float m_Theta;
		float m_Phi;
	};
}
