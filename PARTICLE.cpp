/************************************************************
************************************************************/
#include "PARTICLE.h"
#include <time.h>


/************************************************************
PARTICLE methods
************************************************************/

/******************************
■SJ-magic-study/study__rand
	https://github.com/SJ-magic-study/study__rand
******************************/
PARTICLE::PARTICLE(){
	/********************
	********************/
	position.x = get_Random() * ofGetWidth();
	position.y = get_Random() * ofGetHeight();
	
	velocity = ofVec2f(0.0f, 0.0f);
	force.set(0, 0);
	
	init_Size_ColAlpha();
}

/******************************
******************************/
void PARTICLE::init_Size_ColAlpha(){
	Size.x = ofMap( get_Random(), 0, 1, Gui_Global->Particle_Size_min, Gui_Global->Particle_Size_max );
	Size.y = Size.x;
	
	col_a = ofMap( get_Random(), 0, 1, Gui_Global->Particle_Alpha_min, Gui_Global->Particle_Alpha_max );
}

/******************************
******************************/
void PARTICLE::resetForce(){
	force.set(0, 0);
}

/******************************
******************************/
void PARTICLE::updateVel(int dt){
	velocity = velocity * Gui_Global->ParticleFriction;
	velocity += force * dt;
}

/******************************
******************************/
void PARTICLE::updatePos(int dt){
	position += velocity * dt;
}

/******************************
******************************/
void PARTICLE::update(int dt){
	updateVel(dt);
	updatePos(dt);
	
	throughOfWalls();
}


/******************************
******************************/
void PARTICLE::throughOfWalls(){
	bool b_through = false;
	
	float minx = 0;
	float miny = 0;
	float maxx = ofGetWidth() - 1;
	float maxy = ofGetHeight() - 1;
	if (position.x < minx) {
		position.x = maxx + position.x;
		b_through = true;
	}
	if (position.y < miny) {
		position.y = maxy + position.y;
		b_through = true;
	}
	if (maxx < position.x) {
		position.x = position.x - maxx;
		b_through = true;
	}
	if (maxy < position.y) {
		position.y = position.y - maxy;
		b_through = true;
	}
	
	if(b_through) init_Size_ColAlpha();
}

/******************************
description
	引き付けあう力
	
param
	scale
		中心からの距離によってForceが決まるが、これに対するscaling.
		時間は無関係である点に注意。
******************************/
void PARTICLE::addAttractionForce(float x, float y, float radius)
{
	/********************
	********************/
	ofVec2f posOfForce(x, y);
	
	ofVec2f diff = position - posOfForce;
	float length = diff.length();

	bool b_OutOfRange = false;
	if ( (0 < radius) && (radius < length) )	b_OutOfRange = true;

	if (!b_OutOfRange){
		// 距離から点にかかる力ベクトルを計算
		float pct = 1 - (length / radius);
		diff.normalize();
		force.x = - diff.x  * pct * Gui_Global->ParticleForceScale;
		force.y = - diff.y  * pct * Gui_Global->ParticleForceScale;
	}
}

/************************************************************
PARTICLE_SET methods
************************************************************/
/******************************
******************************/
PARTICLE_SET::PARTICLE_SET()
: atraction(false)
{
	/********************
	注)	謝って、PARTICLE()側にsrand()を置かないこと
		PARTICLEの生成で、都度 srand()が入り、乱数の結果が全て同じ(重なる)になってしまう。
	********************/
	srand((unsigned int)time(NULL));
	
}

/******************************
******************************/
PARTICLE_SET::~PARTICLE_SET()
{
}

/******************************
******************************/
void PARTICLE_SET::setup()
{
	/********************
	■C++ std::vector resizeとreserveの違い
		https://suzulang.com/c-stdvector-resize%E3%81%A8reserve%E3%81%AE%E9%81%95%E3%81%84-%E5%8A%A0%E7%AD%86%E4%B8%AD%EF%BC%9F/
	
	■std::vectorのresizeとassignの違い (C++)
		https://minus9d.hatenablog.com/entry/2021/02/07/175159
	********************/
	clouds.resize(NUM_CLOUDS);
	
	/********************
	********************/
	for(int i = 0; i < NUM_PARTICLE_TEXTURES; i++){
		char buf[512];
		sprintf(buf, "texture/particleTexture_%d.png", i);
		TextureImg[i].load(buf);
	}
	
	VboSet.setup(NUM_VBOS, true);
	Refresh_vboVerts();
	Refresh_vboColor();
	VboSet.update_vertex_color();
	
	Refresh_vboTexCoords();
	VboSet.update_TexCoords();
	
	
	t_ofs__SingedNoise_for_CenterOfForce.x = get_Random() * 1000.0;
	t_ofs__SingedNoise_for_CenterOfForce.y = get_Random() * 1000.0;
}

/******************************
1--2
|  |
0--3
******************************/
void PARTICLE_SET::Refresh_vboVerts()
{
	for(int i = 0; i < NUM_CLOUDS; i++){
		VboSet.set_vboVerts(i * 4 + 0, clouds[i].getPos_LeftLow().x, clouds[i].getPos_LeftLow().y );
		VboSet.set_vboVerts(i * 4 + 1, clouds[i].getPos_LeftUp().x, clouds[i].getPos_LeftUp().y );
		VboSet.set_vboVerts(i * 4 + 2, clouds[i].getPos_RightUp().x, clouds[i].getPos_RightUp().y );
		VboSet.set_vboVerts(i * 4 + 3, clouds[i].getPos_RightLow().x, clouds[i].getPos_RightLow().y );
	}
}

/******************************
colorはSpeed downと共にαが下がっていくようにした。
******************************/
void PARTICLE_SET::Refresh_vboColor()
{
	if(!Gui_Global->b_Particle_Fade){
		for(int i = 0; i < NUM_CLOUDS; i++){
			ofColor col = Gui_Global->Particle_col;
			col.a *= clouds[i].get_ColAlpha();
			
			VboSet.set_vboColor(i * 4 + 0, col);
			VboSet.set_vboColor(i * 4 + 1, col);
			VboSet.set_vboColor(i * 4 + 2, col);
			VboSet.set_vboColor(i * 4 + 3, col);
		}
		
	}else{
		for (int i = 0; i < clouds.size(); i++) {
			ofColor col = Gui_Global->Particle_col;
			col.a *= clouds[i].get_ColAlpha();
			
			const double Alpha_max = double(col.a) / 255;
			double tan = Alpha_max / Gui_Global->Particle_SpeedThresh;
			
			float Alpha;
			if(Gui_Global->Particle_SpeedThresh < clouds[i].get_Speed())	Alpha = Alpha_max;
			else															Alpha = tan * clouds[i].get_Speed();
			
			col.a = 255.0 * Alpha;
			
			VboSet.set_vboColor(i * 4 + 0, col);
			VboSet.set_vboColor(i * 4 + 1, col);
			VboSet.set_vboColor(i * 4 + 2, col);
			VboSet.set_vboColor(i * 4 + 3, col);
		}
	}
}

/******************************
******************************/
void PARTICLE_SET::Refresh_vboTexCoords()
{
	for(int i = 0; i < NUM_CLOUDS; i++){
		VboSet.set_TexCoords(i * 4 + 0, 0, TextureImg[0].getHeight());
		VboSet.set_TexCoords(i * 4 + 1, 0, 0);
		VboSet.set_TexCoords(i * 4 + 2, TextureImg[0].getWidth(), 0);
		VboSet.set_TexCoords(i * 4 + 3, TextureImg[0].getWidth(), TextureImg[0].getHeight());
	}
}

/******************************
■How to use mouseX in a class?
	https://forum.openframeworks.cc/t/how-to-use-mousex-in-a-class/3281
******************************/
void PARTICLE_SET::update()
{
	/********************
	********************/
	static int LastINT = ofGetElapsedTimeMillis();
	int now = ofGetElapsedTimeMillis();
	int dt = ofClamp(now - LastINT, 0, 100);
	
	LastINT = now;
	
	/********************
	********************/
	{
		const float scale = 1.0;
		CenterOfForce.x = ofGetWidth()/2  + ofSignedNoise( double(now)/1000 / T_ForcePos + t_ofs__SingedNoise_for_CenterOfForce.x) * (ofGetWidth() * scale);
		CenterOfForce.y = ofGetHeight()/2 + ofSignedNoise( double(now)/1000 / T_ForcePos + t_ofs__SingedNoise_for_CenterOfForce.y) * (ofGetHeight() * scale);
	}
	
	/********************
	update the position of all particle.
	********************/
	for (int i = 0; i < clouds.size(); i++){
		clouds[i].resetForce();
		if (Gui_Global->b_ParticleAttraction || atraction){
			if(Gui_Global->b_Particle_UseMouse)	clouds[i].addAttractionForce(ofGetAppPtr()->mouseX, ofGetAppPtr()->mouseY, ofGetWidth());
			else								clouds[i].addAttractionForce(CenterOfForce.x, CenterOfForce.y, ofGetWidth());
		}
		
		clouds[i].update(dt);
	}
	
	/********************
	********************/
	Refresh_vboVerts();
	VboSet.update_vertex();
	
	Refresh_vboColor();
	VboSet.update_color();
}

/******************************
******************************/
void PARTICLE_SET::draw()
{
	/********************
	********************/
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	// ofEnableSmoothing();
	
	/********************
	********************/
	ofSetColor(255, 255, 255, 255);
	
	TextureImg[int(std::floor(Gui_Global->Particle_Texture_id))].bind();
	VboSet.draw(GL_QUADS);	
	TextureImg[int(std::floor(Gui_Global->Particle_Texture_id))].unbind();
	/*
	glPointSize(Gui_Global->Particle_PointSize);
	glEnable(GL_POINT_SMOOTH);
	VboSet.draw(GL_POINTS);
	*/
	
	/********************
	********************/
	// 重力の点を描く
	ofFill();
	if (Gui_Global->b_ParticleAttraction || atraction)	ofSetColor(255, 0, 0);
	else												ofSetColor(0, 255, 255);
	
	if(Gui_Global->b_Particle_UseMouse)	ofDrawCircle(ofGetAppPtr()->mouseX, ofGetAppPtr()->mouseY, 4);
	else								ofDrawCircle(CenterOfForce.x, CenterOfForce.y, 4);
}

/******************************
******************************/
void PARTICLE_SET::set_attractive(bool b_val)
{
	atraction = b_val;
}

