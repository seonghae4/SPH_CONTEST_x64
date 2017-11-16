/*
FLUIDS v.3 - SPH Fluid Simulator for CPU and GPU
Copyright (C) 2012. Rama Hoetzlein, http://fluids3.com

Fluids-ZLib license (* see part 1 below)
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. Acknowledgement of the
original author is required if you publish this in a paper, or use it
in a product. (See fluids3.com for details)
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#ifndef DEF_FLUID_SYS
#define DEF_FLUID_SYS

// ��׼��ͷ�ļ�
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <time.h>

// ������Դ��ͷ�ļ�
#include "common\vector.h"
#include "common\xml_settings.h"
#include "common\gl_helper.h"
#include "common\camera3d.h"


// �궨��
#define MAX_PARAM				60         // ��������
#define GRID_UCHAR				0xFF
#define UNDEFINE				0xFFFFFFFF // �趨0xFFFFFFFFΪδ����ֵ

#define RUN_CPU_SPH				0
#define RUN_CPU_PBF				1

// ��������
#define PRUN_MODE				0	
#define PMAXNUM					1	// �����������
#define PEXAMPLE				2	
#define PSIMSIZE				3	  
#define PSIMSCALE				4	// ���ű���
#define PGRID_DENSITY			5	// grid���ܶ�
#define PGRIDSIZEREALSCALE		6	// grid����ʵ���ű���
#define PVISC					7	// ���
#define PRESTDENSITY			8	// �ܶ�
#define PMASS					9	// ����
#define PCOLLISIONRADIUS		10  // ��ײ�뾶
#define PSPACINGREALWORLD		11	// ��ʵ����ļ��
#define PSMOOTHRADIUS			12  // �⻬�뾶
#define PGASCONSTANT			13	// ���庬��
#define PBOUNDARYSTIFF			14	// �߽絯��
#define PBOUNDARYDAMP			15	// �߽�����
#define PACCEL_LIMIT			16  // ���ٶ�����
#define PVEL_LIMIT				17	// �ٶ�����
#define PSPACINGGRAPHICSWORLD	18	// ͼ������ļ��
#define PGROUND_SLOPE			19	// ������б��
#define PFORCE_MIN				20	// ��С��
#define PFORCE_MAX				21	// �����
#define PMAX_FRAC				22	
#define PDRAWMODE				23	// ���Ʒ�ʽ��������
#define PDRAWSIZE				24	
#define PDRAWTEXT				26	// ��������
#define PCLR_MODE				27
#define PPOINT_GRAV_AMT			28	
#define PSTAT_NEIGHCNT			31	
#define PSTAT_NEIGHCNTMAX		32	// ���������������
#define PSTAT_SEARCHCNT			33	
#define PSTAT_SEARCHCNTMAX		34	// ������Χ�����������
#define PSTAT_PMEM				35	
#define PSTAT_GMEM				36	
#define PTIME_INSERT			37	
#define PTIME_SORT				38
#define PTIME_COUNT				39
#define PTIME_PRESS				40
#define PTIME_FORCE				41
#define PTIME_ADVANCE			42
#define PTIME_RECORD			43	
#define PTIME_RENDER			44	
#define PTIME_TOGPU				45	
#define PTIME_FROMGPU			46	
#define PFORCE_FREQ				47	// ����Ƶ��
#define PTIME_OTHER_FORCE		48
#define PTIME_PBF_STEP			49
#define	PDENSITYERRORFACTOR		50
#define PMINLOOPPCISPH			51  // PCI��Сѭ������
#define PMAXLOOPPCISPH			52  // PCI���ѭ������
#define PKERNELSELF				54
#define PINITIALIZEDENSITY		55

// ��������
#define PGRIDVOLUMEMIN			0  // grid�б����С��cell��Ӧ����С�ĵ�
#define PGRIDVOLUMEMAX			1	
#define PBOUNDARYMIN			2	
#define PBOUNDARYMAX			3	
#define PINITPARTICLEMIN		4	
#define PINITPARTICLEMAX		5	
#define PEMIT_POS				6
#define PEMIT_ANG				7
#define PEMIT_DANG				8	
#define PEMIT_SPREAD			9
#define PEMIT_RATE				10
#define PPOINT_GRAV_POS			11	
#define PPLANE_GRAV_DIR			12	

// ��������
#define PPAUSE					0
#define PDEBUG					1	
#define PUSE_CUDA				2	
#define	PUSE_GRID				3	
#define PWRAP_X					4	
#define PWALL_BARRIER			5	
#define PLEVY_BARRIER			6	
#define PDRAIN_BARRIER			7	
#define PPLANE_GRAV_ON			11	
#define PPROFILE				12
#define PCAPTURE				13	
#define PDRAWGRIDCELLS			14  
#define PPRINTDEBUGGINGINFO		15
#define PDRAWDOMAIN				16
#define	PDRAWGRIDBOUND			17
#define PUSELOADEDSCENE			18

// ȫ�ֱ���
const int grid_adj_cnt = 27;


// �������ݽṹ
struct Particle
{
	Vector3DF pos;                                  // λ��
	Vector3DF deta_pos;                             // PBF�е�λ��������
	Vector3DF vel;                                  // ˲ʱ�ٶ�
	Vector3DF vel_eval;                             // ƽ���ٶ�
	Vector3DF force;                                // ����
	float     pressure;	                            // ѹ��(�޷���)
	float     density;                              // �ܶ�
	float     mass;                                 // ����
	int	      particle_grid_cell_index;             // �������������
	int       next_particle_index_in_the_same_cell;	// ͬһ����������һ�����ӵ�����
	DWORD	  clr;									// ������Ⱦ
};

// �������ݽṹ
struct Cell
{
	uint first_particle_index;   // ÿ��cell�е�һ�����ӵ�����
	uint particle_number;        // ÿ��cell�����ӵ�����
	Cell() : first_particle_index(UNDEFINE), particle_number(0){}
	Cell(uint index, uint number) : first_particle_index(index), particle_number(number){}
};


// ���������淶:
// set��get������ͷ��һ����ĸСд����������ÿ�����ʿ�ͷ��ĸ��д
//     ���ຯ����ͷ��һ����ĸ��д����������ÿ�����ʿ�ͷ��ĸ��д
//
// ������������:
// ȫ�������ÿ��������_�ָ���е���Сд
// �����ھֲ�����ÿ�����ʿ�ͷ��ĸ��д
class ParticleSystem
{

public:
	ParticleSystem();
	~ParticleSystem();

	// set����
	void         setUp(bool bStart);
	void         setRender();
	inline void  setToggle(int p);
	inline void  setParam(int p, int v);
	inline void  setParam(int p, float v);
	inline float IncParam(int p, float v, float mn, float mx);
	inline void  setVec(int p, Vector3DF v);

	int SelectParticle(int x, int y, int wx, int wy, Camera3D& cam);

	// get����
	std::string      getModeStr();
	inline bool      getToggle(int p);
	inline int       getNumPoints();
	inline Vector3DF getGridRes();
	inline int       getGridTotal();
	inline int       getGridAdjCnt();
	inline float     getParam(int p);
	inline Vector3DF getVec(int p);
	inline float     getDT();

	// ���к���
	void Run();
	void RunCPUSPH();
	void RunCPUPBF();

	// ���ƺ���
	void        Draw(Camera3D& cam, float rad);

private:
	static int const lut_size = 100000;

	float lut_kernel_m4_[lut_size];
	float lut_kernel_pressure_grad_[lut_size];

	std::string	scene_name;
	int	        texture_[1];
	int	        sphere_points;

	// ģ�����
	float     param_[MAX_PARAM];
	Vector3DF vec_[MAX_PARAM];
	bool      toggle_[MAX_PARAM];

	// XML �����ļ�
	XmlSettings	xml;

	// ���ƺ�����ر���
	int	   vbo_[3];
	Image  image;

	// SPH�⻬�˺���ϵ��
	float poly6_kern;
	float lap_kern;
	float spiky_kern;

	// ʱ�䲽��
	float time_;
	float time_step;
	float time_step_sph;
	int	  frame;

	// �����������
	std::vector<Particle> points;
	int	                  points_number;

	// �������ݽṹ---������ر���
	std::vector<Cell>	grid;
	int					grid_total;                           // grid��cell������
	Vector3DI			grid_res;                             // x��y��z�����ϸ��ж��ٸ�cell
	Vector3DF			grid_min;
	Vector3DF			grid_max;
	Vector3DF			grid_size;
	Vector3DF			grid_delta;
	int					grid_neighbor_cell_index_offset_[grid_adj_cnt];
	int					grid_nonempty_cell_number;              // grid�������ӵ�cell������
	int					grid_particle_number;					// grid�����ӵ�����

	// �߽���ײ��ر���
	bool  addBoundaryForce;
	float maxBoundaryForce;
	float boundaryForceFactor;
	float forceDistance;


	// ˽�к���
	// SPH����
	void InsertParticlesCPU();
	void ComputePressureGrid();
	void ComputeForceGrid();
	void AdvanceStepSimpleCollision();
	void ComputeDensity() {}
	void PositionBasedFluid();

	// �������ݽṹ---������غ���
	int       getGridCell(const Vector3DF& pos, Vector3DI& gc);
	int       getGridCell(int p, Vector3DI& gc);
	Vector3DI getCell(int c);

	// �⻬�˺���
	float KernelM4(float dist, float sr);
	float KernelM4Lut(float dist, float sr);
	float KernelPressureGrad(float dist, float sr);
	float KernelPressureGradLut(float dist, float sr);

	void ComputeGasConstAndTimeStep();

	// �߽���ײ����
	Vector3DF BoxBoundaryForce(const uint i);
	void      CollisionHandlingSimScale(Vector3DF& pos, Vector3DF& vel);

	// ���ƺ���
	void DrawGrid();
	void DrawDomain(Vector3DF& domain_min, Vector3DF& domain_max);
	void DrawText();

	void setDefaultParams();
	void setExampleParams(bool bStart);
	void setKernels();
	void setSpacing();
	void setInitParticleVolumeFromFile(const Vector3DF& minVec, const Vector3DF& maxVec);
	void setInitParticleVolume(const Vector3DI& numParticlesXYZ, const Vector3DF& lengthXYZ, const float jitter);
	void setGridAllocate(const float border);

	void AllocateParticlesMemory(int cnt);

	// ��ȡXML�ļ�
	void ParseXML(std::string name, int id, bool bStart);

	void Record(int param, std::string name, mint::Time& start);

	// ���ļ��ж�ȡ����ģ��
	int ReadInFluidParticles(const char* filename, Vector3DF& minVec, Vector3DF& maxVec);

	inline float frand();
};


inline bool ParticleSystem::getToggle(int p) {
	return this->toggle_[p];
}

inline int ParticleSystem::getNumPoints() {
	return this->points_number;
}

inline Vector3DF ParticleSystem::getGridRes() {
	return this->grid_res;
}

inline int ParticleSystem::getGridTotal() {
	return this->grid_total;
}

inline int ParticleSystem::getGridAdjCnt() {
	return grid_adj_cnt;
}

inline float ParticleSystem::getParam(int p) {
	return (float)this->param_[p];
}

inline Vector3DF ParticleSystem::getVec(int p) {
	return this->vec_[p];
}

inline float ParticleSystem::getDT() {
	return this->time_step;
}

inline void ParticleSystem::setToggle(int p) {
	this->toggle_[p] = !this->toggle_[p];
}

inline float ParticleSystem::frand() {
	return rand() / (float)RAND_MAX;
}

inline float ParticleSystem::IncParam(int p, float v, float mn, float mx) {
	param_[p] += v;
	if (param_[p] < mn) param_[p] = mn;
	if (param_[p] > mx) param_[p] = mn;
	return param_[p];
}

inline void ParticleSystem::setParam(int p, float v) {
	param_[p] = v;
}

inline void ParticleSystem::setParam(int p, int v) {
	param_[p] = (float)v;
}

inline void ParticleSystem::setVec(int p, Vector3DF v) {
	vec_[p] = v;
}

#endif