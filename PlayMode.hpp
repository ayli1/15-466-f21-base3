#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	/*
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;

	glm::vec3 get_leg_tip_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	*/

	struct Shiny {
		Scene::Transform *transform;
		int value = 0; // +1 for emerald, +2 for sapphire, +5 for diamond
	};

	Scene::Transform *miner = nullptr;

	Scene::Transform *gem   = nullptr;
	GLenum gem_vertex_type  = GL_TRIANGLES; //what sort of primitive to draw; passed to glDrawArrays
	GLuint gem_vertex_start = 0; //first vertex to draw; passed to glDrawArrays
	GLuint gem_vertex_count = 0; //number of vertices to draw; passed to glDrawArrays

	std::vector< Shiny > shinies; //Vector of all gems currently left in the mine

	float miner_speed = 3.0f;
	float platform_radius = 5.3f;
	float wall_thickness = 1.5f;
	bool in_shaft = false;
	
	std::shared_ptr< Sound::PlayingSample > canary;

	//camera:
	Scene::Camera *camera = nullptr;

};
