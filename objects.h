//Object IDs
enum IDS{PLAYER, BULLET, ENEMY};

//Our player
struct Player
{
  int x;
  int y;
  float width;
  float height;
  int facing;

  float speed;
  float max_speed;
  float acceleration;
  float deceleration;

  float y_velocity;
  float jump_power;
  float gravity;

  enum states{WALKING, FALLING, JUMPING};
  int state;

  enum animations{WALK, SHRINK, STAND};
  enum directions{LEFT, RIGHT};
  
  
  ALLEGRO_BITMAP *sheet[3];
  int frames[3];

  int current_animation;
  int current_frame;
  int delay;
  int frame_count;
  float scale_x;
  float scale_y;
  float rotation;

  ALLEGRO_BITMAP *sprite;
};

Player player;

struct Platform
{
  int x;
  int y;
  int width;
  int height;

  bool alive;

  enum type{NORMAL};
  int type;
};

struct Point
{
  int x;
  int y;
};

struct Rect
{
  Point top_left;
  Point bottom_right;
};

Platform platforms[max_platforms];
int num_platforms = 0;
int next_platform = 0;