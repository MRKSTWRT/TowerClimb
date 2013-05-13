#include <iostream>

#include <allegro5\allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\allegro_image.h>

#include "globals.h"
#include "objects.h"
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

void SpawnPlatform(int x, int y, int width, int height);
void UpdatePlatforms();
void DrawPlatforms();
void RemovePlatform(int id); //"kills" the platform at id in the array
int PlayerCollidePlatforms(); //Returns the index of the platform being collided with or -1 if no collision.

void NewGame(); //Re-initializes everything for a new game

void Destroy(); //Destroy everything when closing

//Objects

int cur_x;
int cur_y;

int main(void)
{
  //Variables
  double oldTime = 0;
  double newTime = 0;
  double delta = 0;

  //Allegro variables
  ALLEGRO_EVENT_QUEUE *event_queue = NULL;
  ALLEGRO_TIMER *timer = NULL;

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

    if (!paused)
    {
      UpdatePlatforms();
      UpdatePlayer();

      if ((player.y + cam.y) < HEIGHT / 3)
        cam.y += 1;
      
      if (scrolling)//If the scrolling has started then move the cam by scroll_speed;
      {
        cam.y += scroll_speed;
      }
      else //Else if the player reaches the threshold start scrolling
      {
        if ((player.y + cam.y) < HEIGHT / 3)
          scrolling = true;
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
  al_set_target_bitmap(cam.screen);
  al_clear_to_color(al_map_rgb(0,0,0));
  
  DrawPlatforms();
  DrawPlayer();

  al_set_target_bitmap(al_get_backbuffer(display));
  al_draw_bitmap(cam.screen, 0, 0, 0);

  al_draw_textf(fonts[0], al_map_rgb(255,0,0), 2, 2, 0, "%i FPS - X: %i - Y: %i", game_fps, cur_x, cur_y);
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
  player.jump_power = 20;

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

void SpawnPlatform(int x, int y, int width, int height)
{
  for (int i = 0; i < max_platforms; ++i)
  {
    if (!platforms[i].alive)
    {
      platforms[num_platforms].x = x;
      platforms[num_platforms].y = y;
      platforms[num_platforms].width = width;
      platforms[num_platforms].height = height;
      platforms[num_platforms].alive = true;

      num_platforms++;

      break;
    }
  }
}

void UpdatePlatforms()
{

}

void DrawPlatforms()
{
  al_set_target_bitmap(cam.screen);

  for (int i = 0; i < max_platforms; ++i)
  {
    if (platforms[i].alive)
    {
      al_draw_filled_rectangle(platforms[i].x - cam.x, platforms[i].y + cam.y, platforms[i].x + platforms[i].width - cam.x, platforms[i].y + platforms[i].height + cam.y, al_map_rgb(255,255,255));
    }
  } 
}

void RemovePlatform(int id)
{
  platforms[id].alive = false;
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

  for (int i = 0; i < num_platforms; ++i)
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

void NewGame()
{
  scrolling = false;

  InitCamera();
  InitPlayer();

  //Spawn the starting platforms
  SpawnPlatform(0, HEIGHT - 25, WIDTH, 25);
  SpawnPlatform(0, HEIGHT - 175, 100, 25);
  SpawnPlatform(125, HEIGHT - 250, 100, 25);
  SpawnPlatform(250, HEIGHT - 325, 100, 25);

  new_game = false;
}

void Destroy()
{
  int i;

  al_destroy_font(fonts[0]);

  for (i = 0; i < 4; ++i)
    al_destroy_bitmap(images[i]);

  al_destroy_bitmap(cam.screen);
}