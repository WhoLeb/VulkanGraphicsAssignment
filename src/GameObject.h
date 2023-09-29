#pragma once

#include "BaseClassDefines.h"
#include "Model.h"
#include "Line.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace assignment
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{};
		glm::vec3 rotation{}; // Tait-Bryans YXZ

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	class GameObject
	{
	public:

		using id_t = unsigned int;

		NO_COPY(GameObject);
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		static GameObject createGameObject()
		{
			static id_t currentId = 0;
			return GameObject{ currentId++ };
		}

		const id_t getId() { return id; }

		std::shared_ptr<Model> model{};
		std::shared_ptr<Line> line{};
		glm::vec3 color{};
		TransformComponent transform{};

	private:
		GameObject(id_t objId) : id(objId) {}

	private:
		id_t id;

	};
}
