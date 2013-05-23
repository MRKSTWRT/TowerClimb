//THe FPS of the game
const int FPS = 60;

//Width and height of the window
const int WIDTH = 400;
const int HEIGHT = 600;

//Maximum number of platforms at any one time
const int max_platforms = 12;

//Maximum number of pickups at any one time
const int max_pickups = 12;

//Number of characters available for saving highscore names
const int num_chars = 28;

//Array of characters for saving highscore names
const char name_chars[num_chars] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.";

//Keeps track of the number of platforms currently alive
int num_platforms = 0;

//Setting this to true will close the game
bool done = false;

//Keeps track of whether or not we need to redraw the screen
bool redraw = true;

//Keeps track of the pressed state of each key, true means key is down
enum KEYS{UP, DOWN, LEFT, RIGHT, X, Z, R, P, S, ENTER};
const int num_keys = 10;
bool keys[num_keys] = {false, false, false, false, false, false, false, false, false, false};
bool old_keys[num_keys] = {false, false, false, false, false, false, false, false, false, false};

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

//When this is true display game over screen
bool game_over = false;

//Set this to true to start the automatic scrolling upwards
bool scrolling = false;

//The current speed of the upward scroll
float scroll_speed = 1;

//The maximum possible scroll speed
float max_scroll_speed = 5;

//Keeps track of the highest point reached so far
int highest = 0;

//Keeps track of the total score
int score = 0;

int coins = 0;

//A bit hacky, keeps track of where zero score should be
int zero;

//Keeps track of the dificulty, increase this to make the game wait longer before spawning a new platform
float dificulty = 1;

//Max dificulty, make sure player can always make the jumps
float max_dificulty = 2;

//The minimum amount of space between each platform
int platform_increment = 96;

//Point for calculating the next platform location
Point platform_spawn;

//Array containing the possible widths for platforms
int platform_widths[11] = {100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200};

//Helper for calculating the next width
int next_width;

//How many pixels to offset the background when drawing
int bg_offset = 0;

//int for fading in the game over screen
int game_over_fade = 0;
int game_over_fade_2 = 0;

//Probability of a pickup being spawned
int pickup_chance;

//True when the user chooses to submit highscore
bool submit_score = false;

//True after a name has been entered
bool name_entered = false;

//The current letter to change in the highscore name
int submit_selection = 0;

//Character selection for name
int score_name[] = {0,0,0};