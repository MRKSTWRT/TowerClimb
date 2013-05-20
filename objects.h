enum pickup_types {COIN};

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

  int health;

  enum states{WALKING, FALLING, JUMPING};
  int state;

  enum animations{STAND, RUN, SKID, JUMP};
  enum directions{LEFT, RIGHT};
  
  
  ALLEGRO_BITMAP *sheet[4];
  int frames[4];

  int current_animation;
  int current_frame;
  int delay;
  int frame_count;
  float scale_x;
  float scale_y;
  float rotation;

  ALLEGRO_BITMAP *sprite;

  Rect hitbox;

  Point bottom_left;
  Point bottom_right;
};

struct Platform
{
  int x;
  int y;
  int width;
  int height;

  bool alive;

  enum type{NORMAL};
  int type;

  int position;

  Rect hitbox;

  ALLEGRO_BITMAP *sprite;
};

struct Camera
{
  int x;
  int y;
  int width;
  int height;
  ALLEGRO_BITMAP *screen;
  Point last;
};

struct Pickup
{
  int x;
  int y;
  int type;
  bool alive;
  ALLEGRO_BITMAP *sheet;
  int frame_count;
  int current_frame;
  int delay;
  int frames;
  Rect hitbox;
};