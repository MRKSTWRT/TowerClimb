//THe FPS of the game
const int FPS = 60;

//Width and height of the window
const int WIDTH = 400;
const int HEIGHT = 600;

//PI Constant
const float PI = 3.141592653589793;

const int max_platforms = 20;

//Setting this to true will close the game
bool done = false;

//Keeps track of whether or not we need to redraw the screen
bool redraw = true;

//Keeps track of the pressed state of each key, true means key is down
enum KEYS{UP, DOWN, LEFT, RIGHT, X, Z, R};
const int num_keys = 7;
bool keys[num_keys] = {false, false, false, false, false, false, false};
bool old_keys[num_keys] = {false, false, false, false, false, false, false};

//Keeps track of the state, changing current_state will switch the state.
enum STATES{GAME, MENU};
int current_state = GAME;

//Used for the FPS counter
float game_time = 0;
int frames = 0;
int game_fps = 0;

int skips = 0;

//The allegro_display
ALLEGRO_DISPLAY *display = NULL;

//Keeps track of if the game is paused or not
bool paused = false;

//Set this to true to reset everything for a new game
bool new_game = true;

//Set this to true to start the automatic scrolling upwards
bool scrolling = false;

//The current speed of the upward scroll
float scroll_speed = 1;

//Keeps track of the current score
int highest = 0;

//A bit hacky, keeps track of where zero score should be
int zero;