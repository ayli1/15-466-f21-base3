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
	} left, right, down, up, tap, play_again;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	struct Shiny {
		Scene::Transform *transform;
		bool mined  = false; // Whether the player has acquired this gem
		int  value  = 0;     // Number of times it needs to be tapped in order to be mined
		int  tapped = 0;     // Number of times this gem has been tapped
		float time_since_last_tap = 0.0f;
	};

	Scene::Transform *miner = nullptr;

	Scene::Transform *gem   = nullptr;
	GLenum gem_vertex_type  = GL_TRIANGLES; //what sort of primitive to draw; passed to glDrawArrays
	GLuint gem_vertex_start = 0; //first vertex to draw; passed to glDrawArrays
	GLuint gem_vertex_count = 0; //number of vertices to draw; passed to glDrawArrays

	std::vector< Shiny > shinies; //Vector of all gems currently left in the mine

	glm::vec3 miner_pos0  = glm::vec3(0.0f, 0.0f, 0.0f); // Miner's original position
	float miner_speed     = 3.0f;
	float platform_radius = 5.3f;
	float wall_thickness  = 1.5f;
	bool in_shaft  = false;
	bool reset     = true;
	bool game_over = false;
	size_t num_shinies = 10;
	int score = 0;
	
	std::shared_ptr< Sound::PlayingSample > canary;
	float time_in_mine  = 0.0f;
	float time_of_death = 0.0f;
	float time_to_leave = 2.0f; // Amount of time player has to get out of the mine after canary stops singing

	//camera:
	Scene::Camera *camera = nullptr;

};
