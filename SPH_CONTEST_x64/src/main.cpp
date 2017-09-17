// ��׼��ͷ�ļ�
#include <time.h>

// ������Դ��ͷ�ļ�
#include "common\mtime.h"
#include "common\gl_helper.h"
#include "common\camera3d.h"
#include "common\common_defs.h"

// �Զ���ͷ�ļ�
#include "fluid_system.h"


// �궨��
#define WINDOW_POS_X  100
#define WINDOW_POS_Y  100

// ������
#define		DRAG_OFF		0				
#define		DRAG_LEFT		1
#define		DRAG_RIGHT		2

// ģʽ����
#define	MODE_CAM      0
#define	MODE_CAM_TO   1
#define	MODE_OBJ      2
#define	MODE_OBJPOS   3
#define	MODE_OBJGRP   4
#define	MODE_LIGHTPOS 5
#define	MODE_DOF      6


// ȫ�ֱ���
static Camera3D	      g_camera;
static ParticleSystem g_fluid_system;

// �ƹ�
static Vector4DF g_light[2];
static Vector4DF g_light_to[2];
static float     g_light_fov = 45.0f;

static Vector3DF g_obj_from;
static Vector3DF g_obj_angles;
static Vector3DF g_obj_dang;

static float g_window_width = 1024;
static float g_window_height = 768;
static int   g_shade = 0;
static bool  g_help = false;
static int	 g_lastX = -1;
static int	 g_lastY = -1;
static int	 g_mode = 0;
static int	 g_dragging = 0;
static int   g_color_mode = 0;


// ���Ƴ���
static void drawScene(float* viewmat, bool bShade) {
	
	if (g_shade <= 1 && bShade) {

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);

		Vector4DF amb, diff, spec;
		float shininess = 5.0;

		glColor3f(1, 1, 1);
		glLoadIdentity();
		glLoadMatrixf(viewmat);

		float pos[4];
		pos[0] = g_light[0].x;
		pos[1] = g_light[0].y;
		pos[2] = g_light[0].z;
		pos[3] = 1;
		amb.Set(0, 0, 0, 1); diff.Set(1, 1, 1, 1); spec.Set(1, 1, 1, 1);
		glLightfv(GL_LIGHT0, GL_POSITION, (float*)&pos[0]);
		glLightfv(GL_LIGHT0, GL_AMBIENT, (float*)&amb.x);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, (float*)&diff.x);
		glLightfv(GL_LIGHT0, GL_SPECULAR, (float*)&spec.x);

		amb.Set(0, 0, 0, 1); diff.Set(.3, .3, .3, 1); spec.Set(.1, .1, .1, 1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (float*)&amb.x);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)&diff.x);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (float*)&spec.x);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, (float*)&shininess);

		glLoadMatrixf(viewmat);

		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0.001);
		for (float x = -1000; x <= 1000; x += 100.0) {
			for (float y = -1000; y <= 1000; y += 100.0) {
				glVertex3f(x, 0.0, y);
				glVertex3f(x + 100, 0.0, y);
				glVertex3f(x + 100, 0.0, y + 100);
				glVertex3f(x, 0.0, y + 100);
			}
		}
		glEnd();

		glColor3f(0.1, 0.1, 0.2);
		glDisable(GL_LIGHTING);

		// draw particles
		g_fluid_system.Draw(g_camera, 0.8);					

	}
	else {
		glDisable(GL_LIGHTING);

		// draw particles
		g_fluid_system.Draw(g_camera, 0.55);			
	}
}

static void draw2D() {

	mint::Time start, stop;

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScalef(2.0 / g_window_width, -2.0 / g_window_height, 1);		// Setup view (0,0) to (800,600)
	glTranslatef(-g_window_width / 2.0, -g_window_height / 2, 0.0);


	float view_matrix[16];
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// ������Ϣ
	if (g_fluid_system.getSelected() != -1) {
		g_fluid_system.DrawParticleInfo();
		return;
	}

	char disp[200];

	glColor4f(1.0, 1.0, 1.0, 1.0);
	strcpy(disp, "Press H for help.");		drawText2D(10, 20, disp);

	glColor4f(1.0, 1.0, 0.0, 1.0);
	strcpy(disp, "");
	if (g_fluid_system.getToggle(PCAPTURE)) strcpy(disp, "CAPTURING VIDEO");
	drawText2D(200, 20, disp);

	if (g_help) {
		sprintf(disp, "Method (f/g):        %s", g_fluid_system.getModeStr().c_str());
		drawText2D(20, 40, disp);

		sprintf(disp, "Num Particles:       %d", g_fluid_system.getNumPoints());
		drawText2D(20, 60, disp);

		sprintf(disp, "Grid Resolution:     %d x %d x %d (%d)", 
			(int)g_fluid_system.getGridRes().x, 
			(int)g_fluid_system.getGridRes().y,
			(int)g_fluid_system.getGridRes().z,
			g_fluid_system.getGridTotal());
		drawText2D(20, 80, disp);

		int nsrch = pow(g_fluid_system.getGridAdjCnt(), 1 / 3.0);

		sprintf(disp, "Grid Search:         %d x %d x %d", nsrch, nsrch, nsrch);
		drawText2D(20, 100, disp);

		sprintf(disp, "Insert Time:         %.3f ms", g_fluid_system.getParam(PTIME_INSERT));
		drawText2D(20, 140, disp);

		sprintf(disp, "Sort Time:           %.3f ms", g_fluid_system.getParam(PTIME_SORT));
		drawText2D(20, 160, disp);

		sprintf(disp, "Count Time:          %.3f ms", g_fluid_system.getParam(PTIME_COUNT));
		drawText2D(20, 180, disp);

		sprintf(disp, "Pressure Time:       %.3f ms", g_fluid_system.getParam(PTIME_PRESS));
		drawText2D(20, 200, disp);

		sprintf(disp, "Force Time:          %.3f ms", g_fluid_system.getParam(PTIME_FORCE));
		drawText2D(20, 220, disp);

		sprintf(disp, "Advance Time:        %.3f ms", g_fluid_system.getParam(PTIME_ADVANCE));
		drawText2D(20, 240, disp);

		sprintf(disp, "other force Time:    %.3f ms", g_fluid_system.getParam(PTIME_OTHER_FORCE));
		drawText2D(20, 260, disp);

		sprintf(disp, "PCI Step Time:       %.3f ms", g_fluid_system.getParam(PTIME_PCI_STEP));
		drawText2D(20, 280, disp);

		float st = 0.0f;
		switch ((int)g_fluid_system.getParam(PRUN_MODE))
		{
		case RUN_CPU_SPH:
			st = g_fluid_system.getParam(PTIME_INSERT) + g_fluid_system.getParam(PTIME_PRESS) + g_fluid_system.getParam(PTIME_FORCE) + g_fluid_system.getParam(PTIME_ADVANCE);
			break;
		case RUN_CUDA_INDEX_SPH:
		case RUN_CUDA_FULL_SPH:
			st = g_fluid_system.getParam(PTIME_INSERT) + g_fluid_system.getParam(PTIME_SORT) + g_fluid_system.getParam(PTIME_PRESS) + g_fluid_system.getParam(PTIME_FORCE) + g_fluid_system.getParam(PTIME_ADVANCE);
			break;
		case RUN_CPU_PCISPH:
			st = g_fluid_system.getParam(PTIME_INSERT) + g_fluid_system.getParam(PTIME_OTHER_FORCE) + g_fluid_system.getParam(PTIME_PCI_STEP) + g_fluid_system.getParam(PTIME_ADVANCE);
			break;
		case RUN_CUDA_INDEX_PCISPH:
		case RUN_CUDA_FULL_PCISPH:
			st = g_fluid_system.getParam(PTIME_INSERT) + g_fluid_system.getParam(PTIME_SORT) + g_fluid_system.getParam(PTIME_OTHER_FORCE) + g_fluid_system.getParam(PTIME_PCI_STEP) + g_fluid_system.getParam(PTIME_ADVANCE);
			break;
		default:
			st = g_fluid_system.getParam(PTIME_INSERT) + g_fluid_system.getParam(PTIME_PRESS) + g_fluid_system.getParam(PTIME_FORCE) + g_fluid_system.getParam(PTIME_ADVANCE);
			break;
		}

		sprintf(disp, "Total Sim Time:      %.3f ms, %.1f fps", st, 1000.0 / st);
		drawText2D(20, 320, disp);

		Vector3DF vol = g_fluid_system.getVec(PGRIDVOLUMEMAX);
		vol -= g_fluid_system.getVec(PGRIDVOLUMEMIN);
		sprintf(disp, "Time Step (dt):        %3.5f", g_fluid_system.getDT());
		drawText2D(20, 360, disp);

		sprintf(disp, "Simulation Scale:      %3.5f", g_fluid_system.getParam(PSIMSCALE));
		drawText2D(20, 380, disp);

		sprintf(disp, "Smooth Radius (m):     %3.5f", g_fluid_system.getParam(PSMOOTHRADIUS));
		drawText2D(20, 400, disp);

		sprintf(disp, "Particle Radius (m):   %3.5f", g_fluid_system.getParam(PCOLLISIONRADIUS));
		drawText2D(20, 420, disp);

		sprintf(disp, "Particle Mass (kg):    %0.5f", g_fluid_system.getParam(PMASS));
		drawText2D(20, 440, disp);

		sprintf(disp, "Rest Density (kg/m^3): %3.5f", g_fluid_system.getParam(PRESTDENSITY));
		drawText2D(20, 460, disp);

		sprintf(disp, "Viscosity:             %3.5f", g_fluid_system.getParam(PVISC));
		drawText2D(20, 480, disp);

		sprintf(disp, "Boundary Stiffness:    %3.5f", g_fluid_system.getParam(PBOUNDARYSTIFF));
		drawText2D(20, 500, disp);

		sprintf(disp, "Boundary Dampening:    %4.5f", g_fluid_system.getParam(PBOUNDARYDAMP));
		drawText2D(20, 520, disp);

		vol = g_fluid_system.getVec(PPLANE_GRAV_DIR);
		
		sprintf(disp, "Gravity:               (%3.2f, %3.2f, %3.2f)", vol.x, vol.y, vol.z);
		drawText2D(20, 540, disp);
	}
}

static void display() {

	mint::Time tstart, tstop;
	mint::Time rstart, rstop;

	tstart.SetSystemTime(ACC_NSEC);

	// ��ʼģ��
	if (!g_fluid_system.getToggle(PPAUSE))
		g_fluid_system.Run();

	// ����֡��
	measureFPS();

	glEnable(GL_DEPTH_TEST);

	rstart.SetSystemTime(ACC_NSEC);
	disableShadows();

	// ���֡����
	if (g_shade <= 1)
		glClearColor(0.1, 0.1, 0.1, 1.0);
	else
		glClearColor(0, 0, 0, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	// ���������
	g_camera.updateMatrices();
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(g_camera.getProjMatrix().GetDataF());

	// ������ά����	
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(g_camera.getViewMatrix().GetDataF());
	drawScene(g_camera.getViewMatrix().GetDataF(), true);

	// ���ƶ�ά������Ϣ
	draw2D();

	if (g_fluid_system.getToggle(PPROFILE))
	{
		rstop.SetSystemTime(ACC_NSEC);
		rstop = rstop - rstart;
		printf("RENDER: %s\n", rstop.GetReadableTime().c_str());
	}

	// ����������
	glutSwapBuffers();
	glutPostRedisplay();

	if (g_fluid_system.getToggle(PPROFILE)) {
		tstop.SetSystemTime(ACC_NSEC); tstop = tstop - tstart;
		printf("TOTAL:  %s, %f fps\n", tstop.GetReadableTime().c_str(), 1000.0 / tstop.GetMSec());
		printf("PERFORMANCE:  %d particles/sec, %d\n", (int)(g_fluid_system.getNumPoints() * 1000.0) / tstop.GetMSec(), g_fluid_system.getNumPoints());
	}
}

static void reshape(int width, int height) {

	// �����Ӵ��ĸ߶�&���
	g_window_width = (float)width;
	g_window_height = (float)height;
	glViewport(0, 0, width, height);
}

static void UpdateEmit() {

	g_obj_from = g_fluid_system.getVec(PEMIT_POS);
	g_obj_angles = g_fluid_system.getVec(PEMIT_ANG);
	g_obj_dang = g_fluid_system.getVec(PEMIT_RATE);
}

static void keyboard_func(unsigned char key, int x, int y) {

	Vector3DF fp = g_camera.getPos();
	Vector3DF tp = g_camera.getToPos();

	switch (key) {
		// ��ͣ
	case ' ':
		g_fluid_system.setToggle(PPAUSE);
		break;
		// ��һ��ģ�ⷽ��
	case 'B':
	case 'b':
		// �л�����3Dģ������/�ֶ����ó���
		g_fluid_system.setToggle(PUSELOADEDSCENE);
		g_fluid_system.setup(false);
		break;
	case 'f':
	case 'F':
		g_fluid_system.IncParam(PRUN_MODE, -1, 0, 5);
		g_fluid_system.setup(false);
		break;
		// ��һ��ģ�ⷽ��
	case 'g':
	case 'G':
		g_fluid_system.IncParam(PRUN_MODE, 1, 0, 5);
		g_fluid_system.setup(false);
		break;
		// ��ʾ/���ؾ�������߽�
	case '1':
		g_fluid_system.setToggle(PDRAWGRIDBOUND);
		break;
		// ��ʾ/���������߽�
	case '2':
		g_fluid_system.setToggle(PDRAWDOMAIN);
		break;
		// ��ʾ/���ؾ�������
	case '3':
		g_fluid_system.setToggle(PDRAWGRIDCELLS);
		break;
		// �ı�������ƶ�ģʽ->ƽ��
	case 'C':
		g_mode = MODE_CAM_TO;
		break;
		// �ı�������ƶ�ģʽ->��ת
	case 'c':
		g_mode = MODE_CAM;
		break;
		// ��ʾ������Ϣ
	case 'h':
	case 'H':
		g_help = !g_help;
		break;
		// �л���Դλ�ÿ���ģʽ
	case 'l':
	case 'L':
		g_mode = MODE_LIGHTPOS;
		break;
	// �ƶ����/���Ӿ�Ч���ϵ�ͬ���ƶ����壩
	case 'a':
	case 'A':
		g_camera.setToPos(tp.x - 1, tp.y, tp.z);
		break;
	case 'd':
	case 'D':
		g_camera.setToPos(tp.x + 1, tp.y, tp.z);
		break;
	case 'w':
	case 'W':
		g_camera.setToPos(tp.x, tp.y - 1, tp.z);
		break;
	case 's':
	case 'S':
		g_camera.setToPos(tp.x, tp.y + 1, tp.z);
		break;
	case 'q':
	case 'Q':
		g_camera.setToPos(tp.x, tp.y, tp.z + 1);
		break;
	case 'z':
	case 'Z':
		g_camera.setToPos(tp.x, tp.y, tp.z - 1);
		break;
		// ��һ������
	case '[':
		g_fluid_system.IncParam(PEXAMPLE, -1, 0, 3);
		g_fluid_system.setup(true);
		UpdateEmit();
		break;
		// ��һ������
	case ']':
		g_fluid_system.IncParam(PEXAMPLE, +1, 0, 3);
		g_fluid_system.setup(true);
		UpdateEmit();
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

static void mouse_click_func(int button, int state, int x, int y) {

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON)
			g_dragging = DRAG_LEFT;
		else if (button == GLUT_RIGHT_BUTTON)
			g_dragging = DRAG_RIGHT;
		g_lastX = x;
		g_lastY = y;
	}
	else if (state == GLUT_UP) {
		g_dragging = DRAG_OFF;
	}
}

static void mouse_move_func(int x, int y) {
	g_fluid_system.SelectParticle(x, y, g_window_width, g_window_height, g_camera);
}

static void mouse_drag_func(int x, int y) {

	int dx = x - g_lastX;
	int dy = y - g_lastY;

	switch (g_mode) {
	case MODE_CAM:
		if (g_dragging == DRAG_LEFT) {
			g_camera.moveOrbit(dx, dy, 0, 0);
		}
		else if (g_dragging == DRAG_RIGHT) {
			g_camera.moveOrbit(0, 0, 0, dy*0.15);
		}
		break;
	case MODE_CAM_TO:
		if (g_dragging == DRAG_LEFT) {
			g_camera.moveToPos(dx*0.1, 0, dy*0.1);
		}
		else if (g_dragging == DRAG_RIGHT) {
			g_camera.moveToPos(0, dy*0.1, 0);
		}
		break;
	case MODE_OBJ:
		if (g_dragging == DRAG_LEFT) {
			g_obj_angles.x -= dx*0.1;
			g_obj_angles.y += dy*0.1;
			printf("Obj Angs:  %f %f %f\n", g_obj_angles.x, g_obj_angles.y, g_obj_angles.z);
		}
		else if (g_dragging == DRAG_RIGHT) {
			g_obj_angles.z -= dy*.005;
			printf("Obj Angs:  %f %f %f\n", g_obj_angles.x, g_obj_angles.y, g_obj_angles.z);
		}
		g_fluid_system.setVec(PEMIT_ANG, Vector3DF(g_obj_angles.x, g_obj_angles.y, g_obj_angles.z));
		break;
	case MODE_OBJPOS:
		if (g_dragging == DRAG_LEFT) {
			g_obj_from.x -= dx*.1;
			g_obj_from.y += dy*.1;
			printf("Obj:  %f %f %f\n", g_obj_from.x, g_obj_from.y, g_obj_from.z);
		}
		else if (g_dragging == DRAG_RIGHT) {
			g_obj_from.z -= dy*.1;
			printf("Obj:  %f %f %f\n", g_obj_from.x, g_obj_from.y, g_obj_from.z);
		}
		g_fluid_system.setVec(PEMIT_POS, Vector3DF(g_obj_from.x, g_obj_from.y, g_obj_from.z));
		break;
	case MODE_LIGHTPOS:
		if (g_dragging == DRAG_LEFT) {
			g_light[0].x -= dx*.1;
			g_light[0].z -= dy*.1;
			printf("Light: %f %f %f\n", g_light[0].x, g_light[0].y, g_light[0].z);
		}
		else if (g_dragging == DRAG_RIGHT) {
			g_light[0].y -= dy*.1;
			printf("Light: %f %f %f\n", g_light[0].x, g_light[0].y, g_light[0].z);
		}
		break;
	}

	if (x < 10 || y < 10 || x > 1000 || y > 700) {
		glutWarpPointer(1024 / 2, 768 / 2);
		g_lastX = 1024 / 2;
		g_lastY = 768 / 2;
	}
	else {
		g_lastX = x;
		g_lastY = y;
	}
}

void idle_func()
{
}

// ��ʼ��openGL
static void glInit() {
	
	// �򿪿���ݣ����ṩ��������ѵĴ���
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// �Ե����ƽ������
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	// ��ֱ�߽���ƽ������
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// �Զ���ν���ƽ������
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// ���õ�ǰʱ���������������
	srand(time(0x0));

	// ��ʼ��������ɫ
	glClearColor(0.49, 0.49, 0.49, 1.0);
	
	// ����ƽ����ɫ��ֵ
	glShadeModel(GL_SMOOTH);

	// ������ɫ׷��
	glEnable(GL_COLOR_MATERIAL);
	
	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);
	
	// ����2D����ӳ��
	glEnable(GL_TEXTURE_2D);

	// �ص�����
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_click_func);
	glutMotionFunc(mouse_drag_func);
	glutIdleFunc(idle_func);

	// ��ʼ�������
	g_camera.setOrbit(Vector3DF(200, 30, 0), Vector3DF(2, 2, 2), 400, 400);
	g_camera.setFov(35);
	g_camera.updateMatrices();

	g_light[0].x = 0;		g_light[0].y = 200;	g_light[0].z = 0; g_light[0].w = 1;
	g_light_to[0].x = 0;	g_light_to[0].y = 0;	g_light_to[0].z = 0; g_light_to[0].w = 1;

	g_light[1].x = 55;		g_light[1].y = 140;	g_light[1].z = 50;	g_light[1].w = 1;
	g_light_to[1].x = 0;	g_light_to[1].y = 0;	g_light_to[1].z = 0;		g_light_to[1].w = 1;

	g_light_fov = 45.0f;

	g_obj_from.x = 0;		g_obj_from.y = 0;		g_obj_from.z = 20;		// emitter
	g_obj_angles.x = 118.7;	g_obj_angles.y = 200;	g_obj_angles.z = 1.0;
	g_obj_dang.x = 1;	g_obj_dang.y = 1;		g_obj_dang.z = 0;

	g_fluid_system.setup(true);
	g_fluid_system.setVec(PEMIT_ANG, Vector3DF(g_obj_angles.x, g_obj_angles.y, g_obj_angles.z));
	g_fluid_system.setVec(PEMIT_POS, Vector3DF(g_obj_from.x, g_obj_from.y, g_obj_from.z));

	g_fluid_system.setParam(PCLR_MODE, g_color_mode);
}

int main(int argc, char **argv) {

	// ��ʼ��glut
	glutInit(&argc, &argv[0]);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowPosition(WINDOW_POS_X, WINDOW_POS_Y);
	glutInitWindowSize(g_window_width, g_window_height);

	// ��������
	glutCreateWindow("SPH_CONTEST_x64");

	// ��ʼ��openGL
	glInit();

	g_fluid_system.setRender();
	
	glutMainLoop();

	return 0;
}