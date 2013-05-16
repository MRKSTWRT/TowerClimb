#include <iostream>

#include <allegro5\allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\allegro_image.h>

#include "objects.h"
#include "globals.h"
#include "assets.h"

//prototypes
void Update(); //Update the current game state, once every frame
void Draw(); //Handles all of the drawing on screen, after Update
void CheckKeys(ALLEGRO_EVENT &ev, bool pressed); //Checks the current up/down state of each key in the keys array

bool JustPressed(int keycode); //Returns true if keycode has just been pressed this frame

void InitCamera();

void InitPlayer(); //Player constructor, initializes all the starting variables etc.
void UpdatePlayer(); //Updates all player logic
void DrawPlayer(); //Draws the player, also handles the animation
void ChangePlayerAnimation(int animation, bool hard); //Changes the current animation, set hard to true to restart the animation

void SpawnPlatform(int x, int y, int width, int height, int id); //Spawns a platform of width*height at x,y. Supply id for insertion or -1 for first available
void UpdatePlatforms();
void DrawPlatforms();
void RemovePlatform(int id); //"kills" the platform at id in the array
int PlayerCollidePlatforms(); //Returns the index of the platform being collided with or -1 if no collision.

void UpdateBackground(); //Updates the current background offset
void DrawBackground(); //Draws the background

void DrawPauseScreen(); //Draws the pause screen

int Rand(int limit);

void NewGame(); //Re-initializes everything for a new game

void Destroy(); //Destroy everything when closing

//Objects
Player player; //The player object
Platform platforms[max_platforms]; //Array containing all of the platforms
Camera cam; //The camera object for rendering the correct part of the screen


//Debug vars
int cur_x;
int cur_y;
int cam_test;

int main(void)
{
  //Variables
  double oldTime = 0;
  double newTime = 0;
  double delta = 0;

  //Allegro variables
  ALLEGRO_EVENT_QUEUE *event_queue = NULL;
  ALLEGRO_TIMER *timer = NULL;
  ALLEGRO_TIMER *rand_timer = NULL;

  //Initialization Functions
  if(!al_init())										//initialize Allegro
    return -1;

  display = al_create_display(WIDTH, HEIGHT);			//create our display object

  if(!display)										//test display object
    return -1;

  //Allegro Module Init
  al_init_primitives_addon();
  al_init_image_addon();
  al_install_keyboard();
  al_init_font_addon();
  al_init_ttf_addon();

  srand(time(NULL)); //Seed random number generator

  al_set_window_title(display, "TowerClimb");

  event_queue = al_create_event_queue();
  timer = al_create_timer(1.0 / FPS);

  //-------Game Init
  //--Load fonts
  fonts[0] = al_load_font("Assets/Fonts/arial.ttf", 16, 0);
  //--Load images
  images[0] = al_load_bitmap("Assets/Images/Mario-Stand.png");
  images[1] = al_load_bitmap("Assets/Images/Mario-Run.png");
  images[2] = al_load_bitmap("Assets/Images/Mario-Skid.png");
  images[3] = al_load_bitmap("Assets/Images/Mario-Jump.png");
  images[4] = al_load_bitmap("Assets/Images/Platform2.png");
  images[5] = al_load_bitmap("Assets/Images/Background.png");
  

  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_timer_event_source(timer));

  al_start_timer(timer);

  while(!done)
  {
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);

    switch(ev.type)
    {
    case ALLEGRO_EVENT_DISPLAY_CLOSE:
      done = true;
      break;
    case ALLEGRO_EVENT_KEY_DOWN:
      CheckKeys(ev, true);
      break;
    case ALLEGRO_EVENT_KEY_UP:
      CheckKeys(ev, false);
      break;
    case ALLEGRO_EVENT_TIMER:
      Update();
      break;
    }
    
    if(redraw && al_is_event_queue_empty(event_queue))
    {
      redraw = false;
      Draw();
    }
  }

  al_destroy_event_queue(event_queue);
  al_destroy_timer(timer);
  al_destroy_display(display);
  Destroy();

  return 0;
}

void Update()
{
  if (current_state == GAME)
  {
    if (new_game)
      NewGame();

    if (!game_over)
    {
      if (!paused)
      {
        UpdateBackground();
        UpdatePlatforms();
        UpdatePlayer();

        //Save the state of the camera to help with the fake background scrolling
        cam.last.x = cam.x;
        cam.last.y = cam.y;

        if ((player.y + cam.y) < HEIGHT / 4)
          cam.y += 3;
      
        if (scrolling)//If the scrolling has started then move the cam by scroll_speed;
        {
          cam.y += scroll_speed;
        }
        else //Else if the player reaches the threshold start scrolling
        {
          if ((player.y + cam.y) < HEIGHT / 4)
            scrolling = true;
        }
      
        //Keeps track of the highest point the player has reached so far
        if (highest < -(player.y - zero))
          highest = -(player.y - zero);

        if (JustPressed(X))
          dificulty += .1;

        if (JustPressed(P))
          paused = true;
      
      }
      else //Game is paused, update pause screen
      {
        if (JustPressed(P))
          paused = false;
      }
    }

    //Variables used for debug, delete later
    cur_x = player.x + cam.x;
    cur_y = player.y + cam.y;

    if (JustPressed(R))
    {
      new_game = true;
    }
  }
  else if (current_state == MENU)
  {
    
  }

  //Updates the current working fps
  frames++;
  if(al_current_time() - game_time >= 1)
  {
    game_time = al_current_time();
    game_fps = frames;
    frames = 0;
  }

  //Copies keys into old_keys for determining JustPressed
  for (int i = 0; i < num_keys; ++i)
  {
    old_keys[i] = keys[i];
  }

  redraw = true;
}

void Draw()
{ 
  al_set_target_bitmap(cam.screen); //Sets the render target to our camera bitmap
  al_clear_to_color(al_map_rgb(0,0,0)); //Clears the screen to black
  
  //Run individual drawing functions
  DrawBackground();
  DrawPlatforms();
  DrawPlayer();

  if (paused)
    DrawPauseScreen();

  al_set_target_bitmap(al_get_backbuffer(display)); //Set render target to our back buffer
  al_draw_bitmap(cam.screen, 0, 0, 0); //Draw the camera to the back buffer

  al_draw_textf(fonts[0], al_map_rgb(255,0,0), 2, 2, 0, "X: %i Y: %i - Score: %i", cur_x,  cur_y, highest);
  al_flip_display();
}

void CheckKeys(ALLEGRO_EVENT &ev, bool pressed)
{
  if(pressed)
  {
    switch(ev.keyboard.keycode)
    {
    case ALLEGRO_KEY_ESCAPE:
      done = true;
      break;
    case ALLEGRO_KEY_UP:
      keys[UP] = true;
      break;
    case ALLEGRO_KEY_DOWN:
      keys[DOWN] = true;
      break;
    case ALLEGRO_KEY_RIGHT:
      keys[RIGHT] = true;
      break;
    case ALLEGRO_KEY_LEFT:
      keys[LEFT] = true;
      break;
    case ALLEGRO_KEY_X:
      keys[X] = true;
      break;
    case ALLEGRO_KEY_Z:
      keys[Z] = true;
      break;
    case ALLEGRO_KEY_R:
      keys[R] = true;
      break;
    case ALLEGRO_KEY_P:
      keys[P] = true;
      break;
    }
  }
  else
  {
    switch(ev.keyboard.keycode)
    {
    case ALLEGRO_KEY_UP:
      keys[UP] = false;
      break;
    case ALLEGRO_KEY_DOWN:
      keys[DOWN] = false;
      break;
    case ALLEGRO_KEY_RIGHT:
      keys[RIGHT] = false;
      break;
    case ALLEGRO_KEY_LEFT:
      keys[LEFT] = false;
      break;
    case ALLEGRO_KEY_X:
      keys[X] = false;
      break;
    case ALLEGRO_KEY_Z:
      keys[Z] = false;
      break;
    case ALLEGRO_KEY_R:
      keys[R] = false;
      break;
    case ALLEGRO_KEY_P:
      keys[P] = false;
      break;
    }
  }
}

bool JustPressed(int i)
{
  if (old_keys[i] == false && keys[i] == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void InitCamera()
{
  cam.x= 0;
  cam.y = 0;
  cam.last.x = 0;
  cam.last.y = 0;
  cam.width = WIDTH;
  cam.height = HEIGHT;
  cam.screen = al_create_bitmap(cam.width, cam.height);
}

void InitPlayer()
{
  player.width = 16;
  player.height = 32;
  player.scale_x = 2;
  player.scale_y = 2;
  player.rotation = 0;
  player.facing = player.RIGHT;

  player.x = 10;
  player.y = (HEIGHT - (player.height * player.scale_y)) - 25;

  player.max_speed = 5;
  player.acceleration = 0.25;
  player.deceleration = 0.2;
  player.speed = 0;

  player.gravity = 8;
  player.y_velocity = player.gravity;
  player.jump_power = 24;

  player.health = 3;

  player.state = player.WALKING;

  player.sheet[0] = images[0];
  player.frames[0] = 1;
  player.sheet[1] = images[1];
  player.frames[1] = 2;
  player.sheet[2] = images[2];
  player.frames[2] = 1;
  player.sheet[3] = images[3];
  player.frames[3] = 1;

  player.current_frame = 0;
  player.current_animation = player.STAND;
  player.frame_count = 0;
  player.delay = 6;

  player.sprite = al_create_bitmap(player.width, player.height);

  zero = HEIGHT - (player.height * player.scale_y) - 25;
}

void UpdatePlayer()
{
  if (player.state == player.WALKING)
  {
	  if (keys[LEFT])
    {
      player.facing = player.LEFT;
      if (player.speed > 0)
      {
        ChangePlayerAnimation(player.SKID, false);
      }
      else
      {
        ChangePlayerAnimation(player.RUN, false);
      }
    }
    else if (keys[RIGHT])
    {
      player.facing = player.RIGHT;
      if (player.speed < 0)
      {
        ChangePlayerAnimation(player.SKID, false);
      }
      else
      {
        ChangePlayerAnimation(player.RUN, false);
      }
    }
    else
    {
      ChangePlayerAnimation(player.STAND, false);
    }

    if (JustPressed(UP))
    {
      player.state = player.JUMPING;
      player.y_velocity = -player.jump_power;
    }
  }

  if (keys[LEFT])
  {
    player.facing = player.LEFT;

    if (player.speed <= 0)
    {
      player.speed -= player.acceleration;
    }
    else
    {
      player.speed -= player.deceleration;
    }

    if (player.speed < -player.max_speed)
      player.speed = -player.max_speed;
  }
  else if (keys[RIGHT])
  {
    player.facing = player.RIGHT;

    if (player.speed >= 0)
    {
      player.speed += player.acceleration;
    }
    else
    {
      player.speed += player.deceleration;
    }

    if (player.speed > player.max_speed)
      player.speed = player.max_speed;
  }
  else
  {
    if (player.speed < 0 && player.speed > -player.deceleration)
    {
      player.speed = 0;
    }
    else if (player.speed > 0 && player.speed < player.deceleration)
    {
      player.speed = 0;
    }

    if (player.speed < 0)
    {
      player.speed += player.deceleration;
    }
    else if (player.speed > 0)
    {
      player.speed -= player.deceleration;
    }
  }
  
  if (keys[LEFT])
    {
      if (player.speed <= 0)
      {
        player.speed -= player.acceleration;
      }
      else
      {
        player.speed -= player.deceleration;
      }

      if (player.speed < -player.max_speed)
        player.speed = -player.max_speed;
    }
    else if (keys[RIGHT])
    {
      if (player.speed >= 0)
      {
        player.speed += player.acceleration;
      }
      else
      {
        player.speed += player.deceleration;
      }

      if (player.speed > player.max_speed)
        player.speed = player.max_speed;
    }
    else
    {
      if (player.speed < 0 && player.speed > -player.deceleration)
      {
        player.speed = 0;
      }
      else if (player.speed > 0 && player.speed < player.deceleration)
      {
        player.speed = 0;
      }

      if (player.speed < 0)
      {
        player.speed += player.deceleration;
      }
      else if (player.speed > 0)
      {
        player.speed -= player.deceleration;
      }
    }
  
  if (player.state == player.JUMPING && !JustPressed(X))
  {
    ChangePlayerAnimation(player.JUMP, false);
    if (player.y_velocity < player.gravity)
    {
      player.y_velocity += 1;
    }
    else if (player.y_velocity >= 0)
    {
      player.state = player.FALLING;
    }
  }
  else if (player.state == player.FALLING)
  {
    if (player.y_velocity < player.gravity)
    {
      ++player.y_velocity;
    }
  }

  //Apply forces to player
  player.y += player.y_velocity;
  player.x += player.speed;

  if (player.x < -(player.width * player.scale_x)) //Wrap player if he goes off the left side
    player.x = WIDTH;

  if (player.x > WIDTH) // Wrap player if he goes off the right side
    player.x = -(player.width * player.scale_x);

  if (PlayerCollidePlatforms() != -1) //Check for a collision
  {
    player.state = player.WALKING; //Change player state to walking
    player.y_velocity = 0; //Kill the downward velocity
    player.y = platforms[PlayerCollidePlatforms()].y - (player.height * player.scale_y); //Make sure the player hasn't sunk into the platform
  }
  else
  {
    if (player.state != player.JUMPING)
    {
      player.state = player.FALLING; //If we're not jumping and we aren't touching the ground then we must be falling
    }
  }
}

void DrawPlayer()
{
  al_set_target_bitmap(player.sprite); //Set the target to the player sprite image
  al_clear_to_color(al_map_rgba(0,0,0,0)); //Clear the sprite to transparent

  if (player.frame_count >= player.delay) //If the delay has passed
  {
    player.frame_count = 0; //Set counter to zero
    ++player.current_frame; //Increment the current frame
    
    if (player.current_frame > player.frames[player.current_animation] - 1) //if we've gone past the last frame
      player.current_frame = 0; //Go back to the first frame
  }
  
  if (!paused)
    ++player.frame_count; //Increment delay counter

  if (player.facing == player.RIGHT)
  {
    al_draw_bitmap_region(player.sheet[player.current_animation], player.current_frame * player.width, 0, player.width, player.height, 0, 0, 0);
  }
  else if (player.facing == player.LEFT)
  {
    al_draw_bitmap_region(player.sheet[player.current_animation], player.current_frame * player.width, 0, player.width, player.height, 0, 0, ALLEGRO_FLIP_HORIZONTAL);
  }

  al_set_target_bitmap(cam.screen);
  
  al_draw_scaled_bitmap(player.sprite, 0, 0, player.width, player.height, player.x - cam.x, player.y + cam.y, player.width * player.scale_x, player.height * player.scale_y, 0);
}

void ChangePlayerAnimation(int animation, bool hard)
{
  if (hard || player.current_animation != animation)
  {
    player.frame_count = 0;
    player.current_frame = 0;
    player.current_animation = animation;
  }
}

void SpawnPlatform(int x, int y, int width, int height, int id)
{
  int count = width / 32;

  if (id == -1)
  {
    for (int i = 0; i < max_platforms; ++i)
    {
      if (!platforms[i].alive)
      {
        platforms[i].x = x;
        platforms[i].y = y;
        platforms[i].width = width;
        platforms[i].height = height;
        platforms[i].alive = true;
        platforms[i].sprite = al_create_bitmap(width, height);

        al_set_target_bitmap(platforms[i].sprite);

        for (int j = 0; j < count + 1; ++j)
        {
          al_draw_bitmap(images[4], j * 32, 0, 0);
        }

        num_platforms++;

        break;
      }
    }
  }
  else
  {
    if (!platforms[id].alive)
    {
      platforms[id].x = x;
      platforms[id].y = y;
      platforms[id].width = width;
      platforms[id].height = height;
      platforms[id].alive = true;
      platforms[id].sprite = al_create_bitmap(width, height);

      al_set_target_bitmap(platforms[id].sprite);

      for (int j = 0; j < count + 1; ++j)
      {
        al_draw_bitmap(images[4], j * 32, 0, 0);
      }

      num_platforms++;
    }
  }
}

void UpdatePlatforms()
{
  int i = 0;
  
  //Loop through the whole platform array
  for (i = 0; i < max_platforms; ++i)
  {
    if (platforms[i].alive)
    {
      //If the platform is 200 pixels off the screen we can consider it useless and remove it
      if (platforms[i].y > (-cam.y + cam.height + 100))
      {
        RemovePlatform(i);
      }
    }
    else //If platform is not alive we make use of it by spawning a new one in it's place
    {
      next_width = platform_widths[Rand(11)];
      next_width -= Rand(50);

      platform_spawn.y -= platform_increment * dificulty;
      platform_spawn.x = ((WIDTH / 5) * Rand(5)) - (next_width / 2) + 50;
      

      SpawnPlatform(platform_spawn.x, platform_spawn.y, next_width, 32, i);
    }
  }
}

void DrawPlatforms()
{
  al_set_target_bitmap(cam.screen);

  for (int i = 0; i < max_platforms; ++i)
  {
    if (platforms[i].alive)
    {
      al_draw_bitmap(platforms[i].sprite, platforms[i].x - cam.x, platforms[i].y + cam.y, 0);
    }
  } 
}

void RemovePlatform(int id)
{
  platforms[id].alive = false;
  al_destroy_bitmap(platforms[id].sprite);
  --num_platforms;
}

int PlayerCollidePlatforms()
{
  Point player_bottom_left;
  Point player_bottom_right;
  Point player_top_left;
  Point player_top_right;

  player_bottom_left.y = player.y + player.height * player.scale_y;
  player_bottom_right.y = player.y + player.height * player.scale_y;
  
  player_bottom_left.x = player.x;
  player_bottom_right.x = player.x + (player.width * player.scale_x);

  player_top_left.y = player.y;
  player_top_right.y = player.y;

  player_top_left.x = player.x;
  player_top_right.x = player.x + (player.width * player.scale_y);

  for (int i = 0; i < max_platforms; ++i)
  {
    if (platforms[i].alive)
    {
      Rect plat;
      plat.top_left.x = platforms[i].x;
      plat.top_left.y = platforms[i].y;
      plat.bottom_right.x = platforms[i].x + platforms[i].width;
      plat.bottom_right.y = platforms[i].y + platforms[i].height;

      if (player.state == player.FALLING || player.state == player.WALKING)
      {
	      if ((player_bottom_left.x >= plat.top_left.x && player_bottom_left.x <= plat.bottom_right.x) && (player_bottom_left.y >= plat.top_left.y && player_bottom_left.y <= plat.bottom_right.y) ||
	            (player_bottom_right.x >= plat.top_left.x && player_bottom_right.x <= plat.bottom_right.x) && (player_bottom_right.y >= plat.top_left.y && player_bottom_right.y <= plat.bottom_right.y))
	      {
	        //player.y = plat.top_left.y - (player.height * player.scale_y);
	        return i;
	      }
      } 
      else if (player.state == player.JUMPING)
      {
        return -1;
      }
    }
  }

  return -1;
}

void UpdateBackground()
{
  bg_offset += cam.y - cam.last.y;
  if (bg_offset > 32)
    bg_offset -= 32;
}

void DrawBackground()
{
  al_set_target_bitmap(cam.screen);

  al_draw_bitmap(images[5], 0, -32 + bg_offset, 0);
}

void DrawPauseScreen()
{
  al_set_target_bitmap(cam.screen);
  al_draw_filled_rectangle(0, 0, WIDTH, HEIGHT, al_map_rgba(0,0,0,200));
}

int Rand(int limit)
{
  return (int)rand()%limit;
}

void NewGame()
{
  int i;

  scrolling = false;
  game_over = false;
  
  highest = 0;
  bg_offset = 0;


  InitCamera();
  InitPlayer();

  //Remove all platforms
  for (i = 0; i < max_platforms; ++i)
  {
    RemovePlatform(i);
  }

  //Spawn the starting platforms
  SpawnPlatform(0, HEIGHT - 25, WIDTH, 32, -1);
  SpawnPlatform(0, HEIGHT - 175, 100, 32, -1);
  SpawnPlatform(125, HEIGHT - 250, 100, 32, -1);
  SpawnPlatform(250, HEIGHT - 325, 100, 32, -1);

  platform_spawn.y = HEIGHT - 325;

  new_game = false;
}

void Destroy()
{
  int i;

  al_destroy_font(fonts[0]);

  for (i = 0; i < 6; ++i)
    al_destroy_bitmap(images[i]);

  al_destroy_bitmap(player.sprite);
  al_destroy_bitmap(cam.screen);
}