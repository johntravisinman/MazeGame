#include <GL/glut.h>
#include <iostream>
#include <math.h>
#include "RGBpixmap.h"

//Window dimensions
const int window_height = 720;
const int window_width = 1080;

//Camera direction
float target_x = -1.0f;
float target_y = 0.0f;
float target_z = 0.0f;

//Camera Position
float eye_x = 100.0f;
float eye_z = 100.0f;

//Camera movement speed
float movement_speed = 2.0f;

//Camera angle
float angle = -1.6f;

//Teapot rotation
float x_angle = 0.0f;
float y_angle = 0.0f;
bool spinning = true;

//Size of each cell in maze grid
GLfloat side_length = 10;

//Pixel maps
RGBpixmap walls;
RGBpixmap cieling;
RGBpixmap floors;
RGBpixmap teapot;

//Show solution
bool show_solution = false;

//2D maze array
//	0   : draw floor and cieling
//	1   : draw full cell
//	2-9 : solution path, each number represents a specefic texture for showing the solution
//	20+ : draw teapot, floor, and cieling
//  24  : goal location
int maze_arr[19][21] = {
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{ 0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0},
	{ 0,  0,  1,  1,  0,  0,  0,  1,  0,  0,  0,  0,  1,  0,  1,  8,  5,  9,  1,  0,  0},
	{ 0,  0,  1,  0,  0,  1,  0,  1,  0,  1,  0,  1,  0,  0,  1,  4,  1,  4,  1,  0,  0},
	{ 0,  0,  1,  0,  1,  0,  0,  0,  0,  1,  8,  5, 20,  1,  1,  4,  1,  4,  1,  0,  0},
	{ 0,  0,  1,  1,  1,  0,  1,  1,  1,  1,  4,  1,  6,  5,  5,  7,  1,  4,  1,  0,  0},
	{ 0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  4,  1,  1,  1,  1,  1,  1,  4,  1,  0,  0},
	{ 0,  0,  1,  0,  1,  1,  1,  0,  1,  1,  4,  1,  1,  0,  0,  0,  0,  4,  1,  0,  0},
	{ 0,  0,  1,  1,  0,  0,  0,  0,  1,  0,  0,  0,  1,  0,  1,  1,  1,  4,  1,  0,  0},
	{ 0,  0,  1,  1,  0,  1,  1,  1,  1,  0,  0,  0,  1,  0,  0,  0,  1,  4,  1,  0,  0},
	{ 0,  0,  1,  0,  0,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  4,  1,  0,  0},
	{ 0,  0,  1,  1,  1,  1,  0,  0,  8,  5,  9,  1,  0,  0,  0,  8,  5, 21,  1,  0,  0},
	{ 0,  0,  1,  0,  0,  0,  0,  1,  4,  1,  4,  1,  0,  1,  1,  4,  1,  0,  1,  0,  0},
	{ 0,  0,  1,  0,  1,  1,  1,  1,  4,  1,  4,  1,  1,  8, 22,  7,  1,  0,  1,  0,  0},
	{ 0,  0,  1,  1,  8,  5,  5,  5, 23,  1,  6,  5,  5,  7,  1,  0,  1,  0,  1,  0,  0},
	{ 0,  0,  1,  1,  4,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0},
	{ 0,  0,  0,  0, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
}; 

//returns true if there will be a collision with a wall
bool wall_collision(GLfloat box_pos_x, GLfloat box_pos_z, GLfloat next_x_pos, GLfloat next_z_pos) {
	float box_buffer = (side_length / 2) + 0.15;
	if (next_x_pos <= box_pos_x + box_buffer && next_x_pos >= box_pos_x - box_buffer && next_z_pos <= box_pos_z + box_buffer && next_z_pos >= box_pos_z - box_buffer) {
		return true;
	}
	return false;
}

//returns true if there will be a collision with a teapot
bool teapot_collision(GLfloat pot_pos_x, GLfloat pot_pos_z, GLfloat next_x_pos, GLfloat next_z_pos) {
	float teapot_buffer = 1.5f;
	if (next_x_pos <= pot_pos_x + teapot_buffer && next_x_pos >= pot_pos_x - teapot_buffer && next_z_pos <= pot_pos_z + teapot_buffer && next_z_pos >= pot_pos_z - teapot_buffer) {
		return true;
	}
	return false;
}

//handles camera control
void SpecialInput(int key, int t, int y)
{
	//Center coordinate for each cell
	GLfloat box_x = 0.0f;
	GLfloat box_z = 0.0f;
	bool colliding = false; //true if colliding at all
	bool colliding_x = false; //true if slide in x would cause collision
	bool colliding_z = false; //true if slide in z would cause collision
	switch (key)
	{

	case (GLUT_KEY_UP):
		//Iterate through each cell of the maze
		for (auto& row : maze_arr) {
			for (auto& element : row) {
				//If cell is drawn check for wall collisions
				if (element == 1) {
					float next_x_pos = eye_x + target_x * movement_speed;
					float next_z_pos = eye_z + target_z * movement_speed;
					//collisions for standard movement
					if (wall_collision(box_x, box_z, next_x_pos, next_z_pos)) {
						colliding = true;
					}
					//collisions for sliding in x
					if (wall_collision(box_x, box_z, next_x_pos, eye_z)) {
						colliding_x = true;
					}
					//collisions for sliding in z
					if (wall_collision(box_x, box_z, eye_x, next_z_pos)) {
						colliding_z = true;
					}
				}
				//If teapot is drawn check for teapot collisions
				//works the same as checking for wall collisions
				else if (element >= 20 && element != 24) {
					float next_x_pos = eye_x + target_x * movement_speed;
					float next_z_pos = eye_z + target_z * movement_speed;
					if (teapot_collision(box_x, box_z, next_x_pos, next_z_pos)) {
						colliding = true;
					}
					if (teapot_collision(box_x, box_z, next_x_pos, eye_z)) {
						colliding_x = true;
					}
					if (teapot_collision(box_x, box_z, eye_x, next_z_pos)) {
						colliding_z = true;
					}
				}
				//If goal cell check for collision
				else if (element == 24) {
					//If there is collision then we are in goal cell so indicate winner by changing wall textures
					if (wall_collision(box_x, box_z, eye_x, eye_z)) {
						walls.readBMPFile("winner.bmp");
						walls.setTexture(1);
					}
				}
				box_z += side_length;
			}
			box_x += side_length;
			box_z = 0;
		}
		//If no collisions allow standard movement
		if (!colliding) {
			eye_x += target_x * movement_speed;
			eye_z += target_z * movement_speed;
			break;
		}
		//else check if we can slide in x or z direction
		else if (!colliding_x) {
			eye_x += target_x * movement_speed;
		}
		else if (!colliding_z) {
			eye_z += target_z * movement_speed;
		}
		break;

	case GLUT_KEY_DOWN:
		//Functions the same as KEY_UP but the next position is behind instead of in front
		for (auto& row : maze_arr) {
			for (auto& element : row) {
				if (element == 1) {
					float next_x_pos = eye_x - target_x * movement_speed;
					float next_z_pos = eye_z - target_z * movement_speed;
					if (wall_collision(box_x, box_z, next_x_pos, next_z_pos)) {
						colliding = true;
					}
					if (wall_collision(box_x, box_z, next_x_pos, eye_z)) {
						colliding_x = true;
					}
					if (wall_collision(box_x, box_z, eye_x, next_z_pos)) {
						colliding_z = true;
					}
				}
				else if (element >= 20) {
					float next_x_pos = eye_x - target_x * movement_speed;
					float next_z_pos = eye_z - target_z * movement_speed;
					if (teapot_collision(box_x, box_z, next_x_pos, next_z_pos)) {
						colliding = true;
					}
					if (teapot_collision(box_x, box_z, next_x_pos, eye_z)) {
						colliding_x = true;
					}
					if (teapot_collision(box_x, box_z, eye_x, next_z_pos)) {
						colliding_z = true;
					}
				}
				else if (element == 3) {
					if (wall_collision(box_x, box_z, eye_x, eye_z)) {
						walls.readBMPFile("winner.bmp");
						walls.setTexture(1);
					}
				}
				box_z += side_length;
			}
			box_x += side_length;
			box_z = 0;
		}
		if (!colliding) {
			eye_x -= target_x * movement_speed;
			eye_z -= target_z * movement_speed;
			break;
		}
		else if (!colliding_x) {
			eye_x -= target_x * movement_speed;
		}
		else if (!colliding_z) {
			eye_z -= target_z * movement_speed;
		}
		break;
	case GLUT_KEY_LEFT:
		//Move camera target to turn camera along x-z plane
		//anti clockwise rotation
		angle -= 0.1f;
		target_x = sin(angle);
		target_z = -cos(angle);
		break;
	case GLUT_KEY_RIGHT:
		//Move camera target to turn camera along x-z plane
		//clockwise rotation
		angle += 0.1f;
		target_x = sin(angle);
		target_z = -cos(angle);
		break;
	}
	glutPostRedisplay();
}

//function to spin the teapots
void my_anim(void) {
	if (spinning) {
		GLfloat rot_speed = 0.75f;
		x_angle += rot_speed;
		y_angle += rot_speed;
		glutPostRedisplay();
	}
}

//Draws four squars to form a box with no top or bottom (square tube)
void draw_square_tube(GLfloat size) {
	GLfloat x = size / 2;
	GLfloat y = size / 2;
	GLfloat z = size / 2;
	//Front ( + z )
	glBindTexture(GL_TEXTURE_2D, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-x,-y, z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, y, z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( x, y, z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( x,-y, z);
	glEnd();
	//Back ( - z )
	glBindTexture(GL_TEXTURE_2D, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y, -z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-x,  y, -z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y, -z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -y, -z);
	glEnd();
	//Left ( - x )
	glBindTexture(GL_TEXTURE_2D, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y, -z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-x,  y, -z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-x,  y,  z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-x, -y,  z);
	glEnd();
	//Right ( + x )
	glBindTexture(GL_TEXTURE_2D, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f( x, -y, -z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f( x,  y, -z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y,  z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -y,  z);
	glEnd();
}

//Works similar to square_tube except it only draws the floor and cieling and accepts arguments for which texture to use
void draw_floor_and_cieling(GLfloat size, int cieling_texture, int floor_texture) {
	GLfloat x = size / 2;
	GLfloat y = size / 2;
	GLfloat z = size / 2;
	////Cieling ( + y )
	glBindTexture(GL_TEXTURE_2D, cieling_texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, y, -z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, y,  z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( x, y,  z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( x, y, -z);
	glEnd();
	//Floor ( - y )
	glBindTexture(GL_TEXTURE_2D, floor_texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y, -z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, -y,  z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f( x, -y,  z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -y, -z);
	glEnd();
}

//All of the drawing begins here
//Function iterates through the maze_arr and what is drawn is determined by what each element equals
//	0   : draw floor and cieling
//	1   : draw full cell
//	2-9 : solution path, each number represents a specefic texture for showing the solution
//	20+ : draw teapot, floor, and cieling
//  24  : goal location
void draw_maze() {
	//Current center position
	GLint x_pos = 0;
	GLint z_pos = 0;
	for (auto& row : maze_arr) {
		for (auto& element : row) {
			if (element == 1) {
				glPushMatrix();
				glTranslated(x_pos, 0, z_pos);
				draw_square_tube(side_length);
				glPopMatrix();
			}
			if (element >= 20) {
				glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, element);
				glTranslated(x_pos, 0, z_pos);
				glRotated(x_angle, 1.0, 0.0, 0.0);
				glRotated(y_angle, 0.0, 1.0, 0.0);
				glutSolidTeapot(1.0);
				glPopMatrix();
			}
			glPushMatrix();
			glTranslated(x_pos, 0, z_pos);
			//If user has turned on the solution then the following ifs will draw the correct floor textures
			//Note that each floor texture = element + 10
			if (show_solution && element == 4) {
				draw_floor_and_cieling(side_length, 3, 14);
			}
			else if (show_solution && element == 5) {
				draw_floor_and_cieling(side_length, 3, 15);
			}
			else if (show_solution && element == 6) {
				draw_floor_and_cieling(side_length, 3, 16);
			}
			else if (show_solution && element == 7) {
				draw_floor_and_cieling(side_length, 3, 17);
			}
			else if (show_solution && element == 8) {
				draw_floor_and_cieling(side_length, 3, 18);
			}
			else if (show_solution && element == 9) {
				draw_floor_and_cieling(side_length, 3, 19);
			}
			else {
				draw_floor_and_cieling(side_length, 3, 2);
			}
			glPopMatrix();
			z_pos += side_length;
		}
		x_pos += side_length;
		z_pos = 0;
	}
}

//Function to load various textures used in this program
void load_textures() {
	glEnable(GL_TEXTURE_2D);
	walls.readBMPFile("images\\walls.bmp");
	walls.setTexture(1);
	floors.readBMPFile("images\\spotty.bmp");
	floors.setTexture(2);
	cieling.readBMPFile("images\\stars.bmp");
	cieling.setTexture(3);
	floors.readBMPFile("images\\spottyEW.bmp");
	floors.setTexture(14);
	floors.readBMPFile("images\\spottyNS.bmp");
	floors.setTexture(15);
	floors.readBMPFile("images\\spottyNW.bmp");
	floors.setTexture(16);
	floors.readBMPFile("images\\spottySW.bmp");
	floors.setTexture(17);
	floors.readBMPFile("images\\spottyNE.bmp");
	floors.setTexture(18);
	floors.readBMPFile("images\\spottySE.bmp");
	floors.setTexture(19);
	teapot.readBMPFile("images\\red.bmp");
	teapot.setTexture(20);
	teapot.readBMPFile("images\\orange.bmp");
	teapot.setTexture(21);
	teapot.readBMPFile("images\\canary.bmp");
	teapot.setTexture(22);
	teapot.readBMPFile("images\\green.bmp");
	teapot.setTexture(23);
	teapot.readBMPFile("images\\blue.bmp");
	teapot.setTexture(24);


}

//Main display function
void my_display() {
	
	//LIGHT
	GLfloat lightIntensity[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_position[] = { 80.0f, 5.0f, 360.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	//CAMERA
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, window_width / window_height, 0.1, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye_x, 1.0, eye_z, eye_x + target_x, 1.0 + target_y, eye_z + target_z, 0.0, 1.0, 0.0); //movable

	//DRAWING
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	draw_maze();

	glFlush();
	glutSwapBuffers();
}

//Keyboard function to implement non-movement based controls
void my_keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	//	q : quit game
	case 'q':
		exit(-1);
	
	//	. : increase movement speed
	case '.':
		movement_speed += 0.1;
		if (movement_speed > 4)
			movement_speed = 4;
		break;

	//	, : decrease movement speed
	case ',':
		movement_speed -= 0.1;
		if (movement_speed < 0.1)
			movement_speed = 0.1;
		break;

	// r : restart the game
	case 'r':
		eye_x = 100.0f;
		eye_z = 100.0f;
		target_x = -1.0f;
		target_y = 0.0f;
		target_z = 0.0f;
		angle = -1.6f;
		load_textures();
		show_solution = false;
		break;
	
	// space : pause/resume teapot animation
	case ' ':
		spinning = !spinning;
		break;

	// h : show solution
	case 'h':
		show_solution = !show_solution;
		break;
	default:
		break;
	}
}

void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Maze");
	glutDisplayFunc(my_display);
	glutSpecialFunc(SpecialInput);
	glutKeyboardFunc(my_keyboard);
	glutIdleFunc(my_anim);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glClearColor(0.5, 0.0, 0.5, 1.0);
	load_textures();
	std::cout << std::endl << std::endl;
	std::cout << "************************************" << std::endl;
	std::cout << "*********     CONTROLS     *********" << std::endl;
	std::cout << "************************************" << std::endl;
	std::cout << "  arrow keys : movement" << std::endl;
	std::cout << "  spacebar   : pause/resume teapots" << std::endl;
	std::cout << "      ,      : slower movement" << std::endl;
	std::cout << "      .      : faster movement" << std::endl;
	std::cout << "      r      : restart" << std::endl;
	std::cout << "      h      : show solution" << std::endl;
	std::cout << "      q      : quit" << std::endl;
	std::cout << "************************************" << std::endl;
	glutMainLoop();
}