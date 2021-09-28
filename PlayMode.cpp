#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint mine_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > mine_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("mine.pnct"));
	mine_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > mine_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("mine.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = mine_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = mine_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > canary_sample(LoadTagDefault, []() -> Sound::Sample const * {
	//return new Sound::Sample(data_path("dusty-floor.opus"));
	return new Sound::Sample(data_path("canary.wav"));
});

PlayMode::PlayMode() : scene(*mine_scene) {
	//get pointers to leg for convenience:
	/*
	for (auto &transform : scene.transforms) {
		if (transform.name == "Hip.FL") hip = &transform;
		else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
		else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	}
	if (hip == nullptr) throw std::runtime_error("Hip not found.");
	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	hip_base_rotation = hip->rotation;
	upper_leg_base_rotation = upper_leg->rotation;
	lower_leg_base_rotation = lower_leg->rotation;
	*/

	//get pointerz
	/*
	for (auto& transform : scene.transforms) {
		if (transform.name == "Miner")    miner = &transform;
		else if (transform.name == "Gem") gem = &transform;
	}*/

	for (auto& drawable : scene.drawables) {
		if (drawable.transform->name == "Miner") {
			miner = (drawable.transform);
			miner_pos0 = miner->position;
		}
		else if (drawable.transform->name == "Gem") {
			gem = (drawable.transform);
			gem_vertex_type = drawable.pipeline.type;
			gem_vertex_start = drawable.pipeline.start;
			gem_vertex_count = drawable.pipeline.count;
		}
	}

	if (miner == nullptr) throw std::runtime_error("Miner not found.");
	if (gem   == nullptr) throw std::runtime_error("Gem not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	/*
	leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
	*/

	//canary = Sound::play(*canary_sample);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			tap.downs += 1;
			tap.pressed = true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			play_again.downs += 1;
			play_again.pressed = true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			tap.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			play_again.pressed = false;
			if (game_over) reset = true;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	/*
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);

	hip->rotation = hip_base_rotation * glm::angleAxis(
		glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
		glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	//move sound to follow leg tip position:
	leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);
	*/

	//move camera:
	/*
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}
	

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}*/

	// Move miner
	if (!game_over) {
		if (left.pressed && !right.pressed)  miner->position.x -= miner_speed * elapsed;
		if (!left.pressed && right.pressed)  miner->position.x += miner_speed * elapsed;
		if (down.pressed && !up.pressed)     miner->position.y -= miner_speed * elapsed;
		if (!down.pressed && up.pressed)     miner->position.y += miner_speed * elapsed;

		// Bound movement
		if (miner->position.y < -platform_radius) {
			miner->position.y = -platform_radius;
		}
		else if (miner->position.y > (platform_radius - wall_thickness)) {
			miner->position.y = platform_radius - wall_thickness;
		}

		if (!in_shaft) {
			if (miner->position.y > -3.0f) { // Entering the mine shaft
				in_shaft = true;
			}

			// Can only stay within this entrance "pathway"
			if (miner->position.x < -0.5f) {
				miner->position.x = -0.5f;
			}
			else if (miner->position.x > 0.5f) {
				miner->position.x = 0.5f;
			}
		}
		else {
			if (miner->position.y < -3.0f) {
				if ((miner->position.x < -0.5f) || (miner->position.x > 0.5f)) {
					miner->position.y = -3.0f;
				}
				else { // Exiting the mine shaft
					in_shaft = false;
				}
			}

			if (miner->position.x < (-platform_radius + wall_thickness)) {
				miner->position.x = -platform_radius + wall_thickness;
			}
			else if (miner->position.x > (platform_radius - wall_thickness)) {
				miner->position.x = platform_radius - wall_thickness;
			}
		}
	}

	// Referenced my game2 code: https://github.com/ayli1/15-466-f21-base2/blob/main/PlayMode.cpp
	if (reset) { // Randomly generate gems
		reset = false;
		game_over = false;
		score = 0;

		// Clear out any remaining gems from the previous round
		size_t remaining_shinies = shinies.size();
		for (size_t i = 0; i < remaining_shinies; i++) {
			Shiny &shiny = shinies[i];

			// Get rid of this item!! We don't want them anymore >:(
			// First remove from drawables...
			// Referenced: https://stackoverflow.com/questions/16445358/stdfind-object-by-member
			std::list< Scene::Drawable >::iterator it;
			it = std::find_if(scene.drawables.begin(), scene.drawables.end(),
				[&](Scene::Drawable& d) { return d.transform == shiny.transform; });
			if (it != scene.drawables.end()) {
				scene.drawables.erase(it);
			}
			else {
				std::cout << "Hey, couldn't find that drawable ??" << std::endl;
			}

			// ... now remove from shinies vector
			// Referenced: https://stackoverflow.com/questions/3385229/c-erase-vector-element-by-value-rather-than-by-position
			//shinies.erase(shinies.begin() + i);
		}

		shinies.clear();
		std::cout << "Number of shinies after resetting: " << shinies.size() << std::endl;
		miner->position = miner_pos0; // Reset player position
		canary = Sound::play(*canary_sample); // Get that canary singin'

		std::cout << "Resetting" << std::endl;

		// Reference for random number generation in a range: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
		std::random_device rd;  // Obtain random number
		std::mt19937 gen(rd()); // Seed the generator

		// Randomize amount of time for which the canary will sing
		std::uniform_real_distribution< float > c_distr(0.0f, 30.0f);
		time_of_death = c_distr(gen);

		for (size_t i = 0; i < num_shinies; i++) {
			Shiny new_shiny;
			new_shiny.transform = new Scene::Transform;
			new_shiny.transform->rotation = gem->rotation;
			new_shiny.transform->scale    = gem->scale;

			std::uniform_int_distribution<> v_distr(1, 5);
			new_shiny.value = v_distr(gen); // Randomize value of gem

			// Randomize position (within mine shaft)
			std::uniform_real_distribution< float > x_distr(-platform_radius + wall_thickness, platform_radius - wall_thickness); // Set range for position.x
			std::uniform_real_distribution< float > y_distr(-3.0f, platform_radius - wall_thickness); // Set range for position.y
			new_shiny.transform->position.x = x_distr(gen);
			new_shiny.transform->position.y = y_distr(gen);
			new_shiny.transform->position.z = miner_pos0.z;

			// Make sure this gem doesn't overlap with other gems
			for (Shiny other_shiny : shinies) {
				while (glm::distance(new_shiny.transform->position, other_shiny.transform->position) < 0.6f) {
					new_shiny.transform->position.x = x_distr(gen);
					new_shiny.transform->position.y = y_distr(gen);
				}
			}

			// Add to drawables
			scene.drawables.emplace_back(new_shiny.transform);

			Scene::Drawable& drawable = scene.drawables.back();
			drawable.pipeline = lit_color_texture_program_pipeline;
			drawable.pipeline.vao   = mine_meshes_for_lit_color_texture_program;
			drawable.pipeline.type  = gem_vertex_type;
			drawable.pipeline.start = gem_vertex_start;
			drawable.pipeline.count = gem_vertex_count;

			shinies.push_back(new_shiny);
		}
		
	}
	else { // Check for gem collisions
		if (!game_over) time_in_mine += elapsed;

		if (time_in_mine >= (time_of_death + time_to_leave)) {
			// Canary has stopped singing, and miner has run out of time to get out
			game_over = true;
			time_in_mine = 0.0f;
		} else if (time_in_mine >= time_of_death) {
			// Canary stops singing...
			canary->stop();
		}

		for (size_t i = 0; i < shinies.size(); i++) {
			Shiny &shiny = shinies[i];
			if ((glm::distance(shiny.transform->position, miner->position) < 0.5f) &&
				 tap.pressed) {
				shiny.time_since_last_tap += elapsed;
				if (shiny.time_since_last_tap > 0.5f) {
					shiny.tapped += 1;
					shiny.time_since_last_tap = 0.0f;
				}

				if (shiny.tapped == shiny.value) {
					score += shiny.value;

					// Get rid of this item!! We don't want them anymore >:(
					// First remove from drawables...
					// Referenced: https://stackoverflow.com/questions/16445358/stdfind-object-by-member
					std::list< Scene::Drawable >::iterator it;
					it = std::find_if(scene.drawables.begin(), scene.drawables.end(),
						[&](Scene::Drawable& d) { return d.transform == shiny.transform; });
					if (it != scene.drawables.end()) {
						scene.drawables.erase(it);
					}
					else {
						std::cout << "Hey, couldn't find that drawable ??" << std::endl;
					}

					// ... now remove from shinies vector
					// Referenced: https://stackoverflow.com/questions/3385229/c-erase-vector-element-by-value-rather-than-by-position
					shinies.erase(shinies.begin() + i);
				}
			}
		}

		if (shinies.empty()) {
			game_over = true;
		}
	}

	//reset button press counters:
	left.downs  = 0;
	right.downs = 0;
	up.downs    = 0;
	down.downs  = 0;
	tap.downs   = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; escape ungrabs mouse; WASD moves miner; score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; escape ungrabs mouse; WASD moves miner; score: " + std::to_string(score),
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		/*
		if (game_over) {
			if (in_shaft) { // No gems for you because you died :(
				lines.draw_text("YOU LOSE. Press r to play again",
					glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
					glm::vec3((drawable_size.x / 2.0f) - 3.0f, drawable_size.y / 2.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0x00, 0x00, 0x00, 0x00));
				float ofs = 2.0f / drawable_size.y;
				lines.draw_text("YOU LOSE. Press r to play again",
					glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
					glm::vec3((drawable_size.x / 2.0f) - 3.0f, drawable_size.y / 2.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			} else {
				lines.draw_text("YOU WON" + std::to_string(score) + "pts. Press r to play again",
					glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
					glm::vec3((drawable_size.x / 2.0f) - 3.0f, drawable_size.y / 2.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0x00, 0x00, 0x00, 0x00));
				float ofs = 2.0f / drawable_size.y;
				lines.draw_text("YOU WON" + std::to_string(score) + "pts. Press r to play again",
					glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + +0.1f * H + ofs, 0.0),
					glm::vec3((drawable_size.x / 2.0f) - 3.0f, drawable_size.y / 2.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
					glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			}
		}*/
	}
	GL_ERRORS();
}

/*
glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
*/