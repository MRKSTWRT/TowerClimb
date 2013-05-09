#include <iostream>

#include <allegro5\allegro.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\allegro_image.h>

#include "globals.h"
#include "objects.h"
#include "assets.h"

Point debug_circle[4];

//prototypes
void Update(); //Update the current game state, once every frame
void Draw(); //Handles all of the drawing on screen, after Update
void CheckKeys(ALLEGRO_EVENT &ev, bool pressed); //Checks the current up/down state of each key in the keys array

bool JustPressed(int keycode); //Returns true if keycode has just been pressed this frame

void InitPlayer(Player &p); //Player constructor, initializes all the starting variables etc.
void UpdatePlayer(Player &p); //Updates all player logic
void DrawPlayer(Player &p); //Draws the player, also handles the animation
void ChangePlayerAnimation(Player &p, int animation, bool hard); //Changes the current animation, set hard to true to restart the animation

void SpawnPlatform(int x, int y, int width, int height);
void UpdatePlatforms();
void DrawPlatforms();
void RemovePlatform(int id); //"kills" the platform at id in the array
int PlayerCollidePlatforms(); //Returns the index of the platform being collided with or -1 if no collision.

void NewGame(); //Re-initialises everything for a new game

void Destroy(); //Destroy everything when closing

//Objects


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
  arial18 = al_load_font("Assets/Fonts/arial.ttf", 16, 0);
  

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
      UpdatePlayer(player);
    }

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

  DrawPlatforms();
  DrawPlayer(player);

  al_draw_filled_circle(debug_circle[0].x, debug_circle[0].y, 1, al_map_rgb(255,0,0));
  al_draw_filled_circle(debug_circle[1].x, debug_circle[1].y, 1, al_map_rgb(255,0,0));
  al_draw_filled_circle(debug_circle[2].x, debug_circle[2].y, 1, al_map_rgb(255,0,0));
  al_draw_filled_circle(debug_circle[3].x, debug_circle[3].y, 1, al_map_rgb(255,0,0));

  al_draw_textf(arial18, al_map_rgb(255,0,0), 2, 2, 0, "%i FPS - Facing: %i - State %i", game_fps, player.facing, player.state);
  al_flip_display();
  al_clear_to_color(al_map_rgb(0,0,0));
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

void InitPlayer(Player &p)
{
  p.x = 100;
  p.y = 100;
  p.width = 16;
  p.height = 32;
  p.scale_x = 2;
  p.scale_y = 2;
  p.rotation = 0;
  p.facing = p.RIGHT;

  p.max_speed = 5;
  p.acceleration = 0.25;
  p.deceleration = 0.5;
  p.speed = 0;

  p.gravity = 8;
  p.y_velocity = p.gravity;
  p.jump_power = 15;

  p.state = p.WALKING;

  p.sheet[0] = al_load_bitmap("Assets/Images/Mario-Stand.png");
  p.frames[0] = 1;
  p.sheet[1] = al_load_bitmap("Assets/Images/Mario-Run.png");
  p.frames[1] = 2;
  p.sheet[2] = al_load_bitmap("Assets/Images/Mario-Skid.png");
  p.frames[2] = 1;
  p.sheet[3] = al_load_bitmap("Assets/Images/Mario-Jump.png");
  p.frames[3] = 1;

  p.current_frame = 0;
  p.current_animation = p.STAND;
  p.frame_count = 0;
  p.delay = 6;

  p.sprite = al_create_bitmap(p.width, p.height);
}

void UpdatePlayer(Player &p)
{
  if (p.state == p.WALKING)
  {
	  if (keys[LEFT])
    {
      p.facing = p.LEFT;
      if (p.speed > 0)
      {
        ChangePlayerAnimation(p, p.SKID, false);
      }
      else
      {
        ChangePlayerAnimation(p, p.RUN, false);
      }
    }
    else if (keys[RIGHT])
    {
      p.facing = p.RIGHT;
      if (p.speed < 0)
      {
        ChangePlayerAnimation(p, p.SKID, false);
      }
      else
      {
        ChangePlayerAnimation(p, p.RUN, false);
      }
    }
    else
    {
      ChangePlayerAnimation(p, p.STAND, false);
    }

    if (JustPressed(X))
    {
      p.state = p.JUMPING;
      p.y_velocity = -p.jump_power;
      p.speed *= 3;
    }
  }

  if (keys[LEFT])
  {
    p.facing = p.LEFT;

    if (p.speed <= 0)
    {
      p.speed -= p.acceleration;
    }
    else
    {
      p.speed -= p.deceleration;
    }

    if (p.speed < -p.max_speed)
      p.speed = -p.max_speed;
  }
  else if (keys[RIGHT])
  {
    p.facing = p.RIGHT;

    if (p.speed >= 0)
    {
      p.speed += p.acceleration;
    }
    else
    {
      p.speed += p.deceleration;
    }

    if (p.speed > p.max_speed)
      p.speed = p.max_speed;
  }
  else
  {
    if (p.speed < 0 && p.speed > -p.deceleration)
    {
      p.speed = 0;
    }
    else if (p.speed > 0 && p.speed < p.deceleration)
    {
      p.speed = 0;
    }

    if (p.speed < 0)
    {
      p.speed += p.deceleration;
    }
    else if (p.speed > 0)
    {
      p.speed -= p.deceleration;
    }
  }
  
  if (keys[LEFT])
    {
      if (p.speed <= 0)
      {
        p.speed -= p.acceleration;
      }
      else
      {
        p.speed -= p.deceleration;
      }

      if (p.speed < -p.max_speed)
        p.speed = -p.max_speed;
    }
    else if (keys[RIGHT])
    {
      if (p.speed >= 0)
      {
        p.speed += p.acceleration;
      }
      else
      {
        p.speed += p.deceleration;
      }

      if (p.speed > p.max_speed)
        p.speed = p.max_speed;
    }
    else
    {
      if (p.speed < 0 && p.speed > -p.deceleration)
      {
        p.speed = 0;
      }
      else if (p.speed > 0 && p.speed < p.deceleration)
      {
        p.speed = 0;
      }

      if (p.speed < 0)
      {
        p.speed += p.deceleration;
      }
      else if (p.speed > 0)
      {
        p.speed -= p.deceleration;
      }
    }
  
  if (p.state == p.JUMPING && !JustPressed(X))
  {
    ChangePlayerAnimation(p, p.JUMP, false);
    if (p.y_velocity < p.gravity)
    {
      p.y_velocity += 1;
    }
    else if (p.y_velocity >= 0)
    {
      p.state = p.FALLING;
    }
  }
  else if (p.state == p.FALLING)
  {
    if (p.y_velocity < p.gravity)
    {
      ++p.y_velocity;
    }
  }

  p.y += p.y_velocity;
  p.x += p.speed;

  if (PlayerCollidePlatforms() != -1)
  {
    p.state = p.WALKING;
    p.y_velocity = 0;
    p.y = platforms[PlayerCollidePlatforms()].y - (p.height * p.scale_y);
  }
  else
  {
    if (p.state != p.JUMPING)
    {
      p.state = p.FALLING;
    }
  }
}

void DrawPlayer(Player &p)
{
  al_set_target_bitmap(p.sprite); //Set the target to the player sprite image
  al_clear_to_color(al_map_rgba(0,0,0,0)); //Clear the sprite to transparent

  if (p.frame_count >= p.delay) //If the delay has passed
  {
    p.frame_count = 0; //Set counter to zero
    ++p.current_frame; //Increment the current frame
    
    if (p.current_frame > p.frames[p.current_animation] - 1) //if we've gone past the last frame
      p.current_frame = 0; //Go back to the first frame
  }
  
  ++p.frame_count; //Increment delay counter

  if (p.facing == p.RIGHT)
  {
    al_draw_bitmap_region(p.sheet[p.current_animation], p.current_frame * p.width, 0, p.width, p.height, 0, 0, 0);
  }
  else if (p.facing == p.LEFT)
  {
    al_draw_bitmap_region(p.sheet[p.current_animation], p.current_frame * p.width, 0, p.width, p.height, 0, 0, ALLEGRO_FLIP_HORIZONTAL);
  }

  al_set_target_bitmap(al_get_backbuffer(display));
  
  al_draw_scaled_bitmap(p.sprite, 0, 0, p.width, p.height, p.x, p.y, p.width * p.scale_x, p.height * p.scale_y, 0);
}

void ChangePlayerAnimation(Player &p, int animation, bool hard)
{
  if (hard || p.current_animation != animation)
  {
    p.frame_count = 0;
    p.current_frame = 0;
    p.current_animation = animation;
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
  for (int i = 0; i < max_platforms; ++i)
  {
    if (platforms[i].alive)
    {
      al_set_target_bitmap(al_get_backbuffer(display));
      al_draw_filled_rectangle(platforms[i].x, platforms[i].y, platforms[i].x + platforms[i].width, platforms[i].y + platforms[i].height, al_map_rgb(255,255,255));
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

  debug_circle[0] = player_bottom_left;
  debug_circle[1] = player_bottom_right;
  debug_circle[2] = player_top_right;
  debug_circle[3] = player_top_left;

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
  InitPlayer(player);

  SpawnPlatform(0, 200, 100, 25);
  SpawnPlatform(100, 250, 100, 25);
  SpawnPlatform(0, 300, WIDTH, 25);

  new_game = false;
}

void Destroy()
{
  int i;

  al_destroy_font(arial18);

  for (i = 0; i < 2; ++i)
    al_destroy_bitmap(player.sheet[i]);
}