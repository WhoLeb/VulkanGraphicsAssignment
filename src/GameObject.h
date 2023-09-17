#pragma once

#include "BaseClassDefines.h"
#include "Model.h"

#include <memory>

namespace assignment
{
	struct Transform2dComponent
	{
		glm::vec2 translation{};
		glm::mat2 mat2() { return glm::mat2(1.f); }

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
		glm::vec3 color{};
		Transform2dComponent transform2d;

	private:
		GameObject(id_t objId) : id(objId) {}

	private:
		id_t id;

	};
}
