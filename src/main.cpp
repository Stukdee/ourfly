#include "raylib.h"
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <random>

#define KEY_CONTROL_UP (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
#define KEY_CONTROL_DOWN (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
#define KEY_CONTROL_LEFT (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
#define KEY_CONTROL_RIGHT (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
#define KEY_CONTROL_SHOT (IsKeyDown(KEY_Z) || IsKeyDown(KEY_M))

/*常量*/
const int PATH_MAXX = 99999;
const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;

/*全局变量*/
float deltaTime = 0.0F;
float littlePlaneGenerationInterval = 0.0F;

/*纹理*/
std::vector <Texture2D> playerTextures;
std::vector <Texture2D> playerShotTextures;
std::vector <Texture2D> littlePlaneTextures;

/*类定义*/
class BasicInfo {
	private:
	Texture2D *texture;
	Rectangle source;
	Rectangle dest;
	Vector2 origin;
	float rotation;
	Color color;
	float ySort;
	public:
	BasicInfo ();
	/*获取*/
	Texture2D *getTexture () const;
	Rectangle getSource () const;
	Rectangle getDest () const;
	Vector2 getOrigin () const;
	float getRotation () const;
	Color getColor () const;
	float getYSort () const;
	/*设置*/
	void setTexture (Texture2D *texture_);
	void setSource (Rectangle source_);
	void setDest (Rectangle dest_);
	void setOrigin (Vector2 origin_);
	void setRotation (float rotation_);
	void setColor (Color color_);
	void setYSort (float ySort_);
};

class CameraObject {
	private:
	Camera2D camera;
	public:
	CameraObject ();
	void update ();
	Camera2D getCamera () const;
};

class Player {
	private:
	Vector2 midPos;
	Vector2 pos;
	Rectangle rect;
	int action;
	float speed;
	float size;
	public:
	BasicInfo basic;
	Player ();
	void update ();
	Vector2 getMidPos () const;
	Rectangle getRect () const;
};

class PlayerShot {
	private:
	Vector2 midPos;
	Vector2 pos;
	Rectangle rect;
	int action;
	float speed;
	float size;
	bool isDie;
	public:
	BasicInfo basic;
	PlayerShot (Vector2 midPos_);
	void update ();
	bool getIsDie () const;
	Rectangle getRect () const;
	void setIsDie (bool isDie_);
};

class LittlePlane {
	private:
	Vector2 midPos;
	Vector2 pos;
	Rectangle rect;
	int action;
	float speed;
	float size;
	bool isDie;
	float rotation;
	public:
	BasicInfo basic;
	LittlePlane (Vector2 midPos_);
	void update ();
	bool getIsDie () const;
	Rectangle getRect () const;
};

/*实例化类*/
std::vector <BasicInfo*> basicInfos;
CameraObject mainCamera;
Player player;
std::vector <PlayerShot*> playerShots;
std::vector <LittlePlane*> littlePlanes;

/*函数声明与定义*/
static bool compareByYSort(const BasicInfo *a,const BasicInfo *b){
	return a -> getYSort() < b -> getYSort();
}

/*函数声明*/
float lerpCorrect (float a,float b,float step,float theMinest);

Vector2 getOffset (float step,float angle);

float getDeltaTime ();

float getFrom (Vector2 a,Vector2 b);

int randint (int a,int b);

float randfloat (float a,float b);

float rotateLerpCorrect (float a,float b,float step,float theMinest);

int main () {
	/*获取绝对路径*/
	char path[PATH_MAXX];
	strncpy(path,GetApplicationDirectory(),PATH_MAXX);
	char* lastSep = strrchr(path,'/');
	if(!lastSep) lastSep = strrchr(path,'\\');
	if(lastSep) *lastSep = '\0';
	printf("素材文件夹位于 -> %s\n",(path + std::string("/assets")).c_str());

	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"ourfly");
	InitAudioDevice();
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	SetTraceLogLevel(LOG_ERROR);

	/*导入素材*/
	playerTextures.push_back(LoadTexture((path + std::string("/assets/player/1.png")).c_str()));
	playerShotTextures.push_back(LoadTexture((path + std::string("/assets/playerShot/1.png")).c_str()));
	littlePlaneTextures.push_back(LoadTexture((path + std::string("/assets/littlePlane/1.png")).c_str()));

	while (!WindowShouldClose()) {
		/*更新*/
		deltaTime = getDeltaTime();

		/*生成更新*/
		littlePlaneGenerationInterval += deltaTime * randfloat(1,10);
		if (littlePlaneGenerationInterval > 10) {
			littlePlaneGenerationInterval = 0.0F;
			littlePlanes.push_back(new LittlePlane(
				(Vector2){
					float(randint(0 - (SCREEN_WIDTH / 2),SCREEN_WIDTH / 2)),
					0 - SCREEN_HEIGHT
				}
			));
		}

		/*object更新*/
		mainCamera.update();

		basicInfos.clear();

		player.update();
		basicInfos.push_back(&(player.basic));

		for (auto it = playerShots.begin();it != playerShots.end();) {
			(*it) -> update();
			if ((*it) -> getIsDie()) {
				delete (*it);
				it = playerShots.erase(it);
				continue;
			}
			basicInfos.push_back(&((*it) -> basic));
			it++;
		}

		for (auto it = littlePlanes.begin();it != littlePlanes.end();) {
			(*it) -> update();
			if ((*it) -> getIsDie()) {
				delete (*it);
				it = littlePlanes.erase(it);
				continue;
			}
			basicInfos.push_back(&((*it) -> basic));
			it++;
		}

		/*开始绘制*/
		BeginDrawing();
		BeginMode2D(mainCamera.getCamera());
		ClearBackground((Color){134,129,136,255});
		if (basicInfos.size() > 1) {
			std::sort(basicInfos.begin(),basicInfos.end(),compareByYSort);
		}
		for (auto it = basicInfos.begin();it != basicInfos.end();it++) {
			DrawTexturePro(
				*((*it) -> getTexture()),
				(*it) -> getSource(),
				(*it) -> getDest(),
				(*it) -> getOrigin(),
				(*it) -> getRotation(),
				(*it) -> getColor()
			);
		}
		EndMode2D();
		/*绘制黑边（保持游戏画面1:1）*/
		float gameW = SCREEN_WIDTH * mainCamera.getCamera().zoom;
		float gameH = SCREEN_HEIGHT * mainCamera.getCamera().zoom;
		float barX = (GetScreenWidth() - gameW) / 2.0F;
		float barY = (GetScreenHeight() - gameH) / 2.0F;
		if (barX > 0) {
			DrawRectangleRec((Rectangle){0,0,barX,(float)GetScreenHeight()},(Color){0,0,0,255});
			DrawRectangleRec((Rectangle){(float)GetScreenWidth() - barX,0,barX,(float)GetScreenHeight()},(Color){0,0,0,255});
		}
		if (barY > 0) {
			DrawRectangleRec((Rectangle){0,0,(float)GetScreenWidth(),barY},(Color){0,0,0,255});
			DrawRectangleRec((Rectangle){0,(float)GetScreenHeight() - barY,(float)GetScreenWidth(), barY},(Color){0,0,0,255});
		}
		EndDrawing();
	}
	/*释放资源*/
	for (auto it = playerTextures.begin();it != playerTextures.end();it++) {
		UnloadTexture(*it);
	}
	for (auto it = playerShotTextures.begin();it != playerShotTextures.end();it++) {
		UnloadTexture(*it);
	}
	for (auto it = littlePlaneTextures.begin();it != littlePlaneTextures.end();it++) {
		UnloadTexture(*it);
	}
	CloseWindow();
	CloseAudioDevice();
	return 0;
}

/*BasicInfo类的实现*/
BasicInfo::BasicInfo () {
	texture = NULL;
	source = (Rectangle){0,0,0,0};
	dest = (Rectangle){0,0,0,0};
	origin = (Vector2){0,0};
	rotation = 0.0F;
	color = (Color){255,255,255,255};
	ySort = 0.0F;
}

Texture2D *BasicInfo::getTexture () const {
	return this -> texture;
}

Rectangle BasicInfo::getSource () const {
	return this -> source;
}

Rectangle BasicInfo::getDest () const {
	return this -> dest;
}

Vector2 BasicInfo::getOrigin () const {
	return this -> origin;
}

float BasicInfo::getRotation () const {
	return this -> rotation;
}

Color BasicInfo::getColor () const {
	return this -> color;
}

float BasicInfo::getYSort () const {
	return this -> ySort;
}

void BasicInfo::setTexture (Texture2D *texture_) {
	this -> texture = texture_;
	return;
}

void BasicInfo::setSource (Rectangle source_) {
	this -> source = source_;
	return;
}

void BasicInfo::setDest (Rectangle dest_) {
	this -> dest = dest_;
	return;
}

void BasicInfo::setOrigin (Vector2 origin_) {
	this -> origin = origin_;
	return;
}

void BasicInfo::setRotation (float rotation_) {
	this -> rotation = rotation_;
	return;
}

void BasicInfo::setColor (Color color_) {
	this -> color = color_;
	return;
}

void BasicInfo::setYSort (float ySort_) {
	this -> ySort = ySort_;
	return;
}

/*CameraObject类的实现*/
CameraObject::CameraObject () {
	(this -> camera).offset = (Vector2){(float)GetScreenWidth() / 2,(float)GetScreenHeight() / 2};
	(this -> camera).rotation = 0;
	(this -> camera).target = (Vector2){0,0};
	(this -> camera).zoom = 1;
}

void CameraObject::update () {
	(this -> camera).offset = (Vector2){(float)GetScreenWidth() / 2,(float)GetScreenHeight() / 2};
	static float scale = 1.0F;
	int currentWidth = GetScreenWidth();
	int currentHeight = GetScreenHeight();
	float scaleX = (float)currentWidth / SCREEN_WIDTH;
	float scaleY = (float)currentHeight / SCREEN_HEIGHT;
	float baseZoom = (scaleX < scaleY) ? scaleX : scaleY;
	(this -> camera).zoom = scale * baseZoom;
	return;
}

Camera2D CameraObject::getCamera () const {
	return (this -> camera);
}

/*Player类的实现*/
Player::Player () {
	(this -> midPos) = (Vector2){0,(SCREEN_HEIGHT / 4)};
	(this -> action) = 0;
	(this -> speed) = 5;
	(this -> size) = 2.0F;
}

Vector2 Player::getMidPos () const {
	return (this -> midPos);
}

Rectangle Player::getRect () const {
	return (this -> rect);
}

void Player::update () {
	/*静态变量*/
	static float shotInterval = 0.0F;

	/*更新间隔*/
	shotInterval += deltaTime;

	/*玩家移动*/
	int stepCount = 0;
	float angle = 0;
	static float lastDirection = 0.0F;
	static Vector2 offsetSpeed = (Vector2){0,0};
	if (KEY_CONTROL_UP) {angle += 270;stepCount++;}
	if (KEY_CONTROL_DOWN) {angle += 90;stepCount++;}
	if (KEY_CONTROL_LEFT) {angle += 180;stepCount++;}
	if (KEY_CONTROL_RIGHT) {angle += 0;stepCount++;}
	if (KEY_CONTROL_UP && KEY_CONTROL_RIGHT) {angle = 315;stepCount = 1;}
	if (KEY_CONTROL_UP && KEY_CONTROL_DOWN) {stepCount = 0;}
	if (KEY_CONTROL_RIGHT && KEY_CONTROL_LEFT) {stepCount = 0;}
	if (stepCount != 0) {
		lastDirection = angle / stepCount;
		offsetSpeed.x = lerpCorrect(offsetSpeed.x,getOffset((this -> speed),lastDirection).x,0.1,0.01);
		offsetSpeed.y = lerpCorrect(offsetSpeed.y,getOffset((this -> speed),lastDirection).y,0.1,0.01);
	}
	else {
		offsetSpeed.x = lerpCorrect(offsetSpeed.x,0,0.1,0.01);
		offsetSpeed.y = lerpCorrect(offsetSpeed.y,0,0.1,0.01);
	}
	if (((this -> midPos).x > (SCREEN_WIDTH / 2)) || ((this -> midPos).x < (0 - (SCREEN_WIDTH / 2)))) {
		offsetSpeed.x = getOffset((this -> speed),(this -> midPos.x) < 0 ? 0.0F : 180.0F).x;
	}
	if (((this -> midPos).y > (SCREEN_HEIGHT / 2)) || ((this -> midPos).y < (0 - (SCREEN_HEIGHT / 2)))) {
		offsetSpeed.y = getOffset((this -> speed),(this -> midPos.y) < 0 ? 90.0F : -90.0F).y;
	}
	(this -> midPos).x += offsetSpeed.x * deltaTime * 60;
	(this -> midPos).y += offsetSpeed.y * deltaTime * 60;
	/*玩家射击*/
	if (KEY_CONTROL_SHOT && (shotInterval > 0.2)) {
		shotInterval = 0.0F;
		playerShots.push_back(new PlayerShot((this -> midPos)));
	}

	/*获取纹理*/
	(this -> basic).setTexture(&(playerTextures.at(this -> action)));

	/*设置位置*/
	(this -> pos) = (Vector2){
		(this -> midPos).x - (float((this -> basic).getTexture() -> width) * (this -> size) / 2),
		(this -> midPos).y - (float((this -> basic).getTexture() -> height) * (this -> size) / 2)
	};

	/*设置basic*/
	(this -> basic).setSource((Rectangle){
		0,0,
		float((this -> basic).getTexture() -> width),
		float((this -> basic).getTexture() -> height)
	});
	(this -> basic).setDest((Rectangle){
		(this -> midPos).x,
		(this -> midPos).y,
		float((this -> basic).getTexture() -> width) * (this -> size),
		float((this -> basic).getTexture() -> height) * (this -> size)
	});
	(this -> basic).setOrigin((Vector2){
		float((this -> basic).getTexture() -> width) / 2 * (this -> size),
		float((this -> basic).getTexture() -> height) / 2 * (this -> size)
	});
	(this -> basic).setRotation(-90.0F);
	(this -> basic).setColor((Color){255,255,255,255});
	(this -> basic).setYSort((this -> pos).y + ((this -> rect).height * (this -> size)));

	/*设置rect*/
	(this -> rect).width = (this -> basic).getTexture() -> width * (this -> size);
	(this -> rect).height = (this -> basic).getTexture() -> height * (this -> size);
	(this -> rect).x = (this -> pos).x;
	(this -> rect).y = (this -> pos).y;
	return;
}

/*PlayerShot类的实现*/
PlayerShot::PlayerShot (Vector2 midPos_) {
	(this -> midPos) = midPos_;
	(this -> action) = 0;
	(this -> speed) = 10;
	(this -> size) = 2.0F;
	(this -> isDie) = false;
}

Rectangle PlayerShot::getRect () const {
	return (this -> rect);
}

void PlayerShot::update () {
	/*移动*/
	(this -> midPos).y -= (this -> speed) * deltaTime * 60;
	if ((this -> midPos).y < -SCREEN_HEIGHT) {
		(this -> isDie) = true;
	}

	/*获取纹理*/
	(this -> basic).setTexture(&(playerShotTextures.at(this -> action)));

	/*设置位置*/
	(this -> pos) = (Vector2){
		(this -> midPos).x - (float((this -> basic).getTexture() -> width) * (this -> size) / 2),
		(this -> midPos).y - (float((this -> basic).getTexture() -> height) * (this -> size) / 2)
	};

	/*设置basic*/
	(this -> basic).setSource((Rectangle){
		0,0,
		float((this -> basic).getTexture() -> width),
		float((this -> basic).getTexture() -> height)
	});
	(this -> basic).setDest((Rectangle){
		(this -> midPos).x,
		(this -> midPos).y,
		float((this -> basic).getTexture() -> width) * (this -> size),
		float((this -> basic).getTexture() -> height) * (this -> size)
	});
	(this -> basic).setOrigin((Vector2){
		float((this -> basic).getTexture() -> width) / 2 * (this -> size),
		float((this -> basic).getTexture() -> height) / 2 * (this -> size)
	});
	(this -> basic).setRotation(-90.0F);
	(this -> basic).setColor((Color){255,255,255,255});
	(this -> basic).setYSort((this -> pos).y + ((this -> rect).height * (this -> size)));

	/*设置rect*/
	(this -> rect).width = (this -> basic).getTexture() -> width * (this -> size);
	(this -> rect).height = (this -> basic).getTexture() -> height * (this -> size);
	(this -> rect).x = (this -> pos).x;
	(this -> rect).y = (this -> pos).y;
	return;
}

bool PlayerShot::getIsDie () const {
	return (this -> isDie);
}

void PlayerShot::setIsDie (bool isDie_) {
	(this -> isDie) = isDie_;
	return;
}

/*LittlePlane类的实现*/
LittlePlane::LittlePlane (Vector2 midPos_) {
	(this -> midPos) = midPos_;
	(this -> action) = 0;
	(this -> speed) = 5;
	(this -> size) = 2.0F;
	(this -> rotation) = 0.0F;
	(this -> isDie) = false;
}

void LittlePlane::update () {
	/*更随玩家*/
	(this -> rotation) = rotateLerpCorrect(
		(this -> rotation),
		getFrom((this -> midPos),player.getMidPos()),
		0.05 * deltaTime * 60,
		0.1
	);
	(this -> midPos).x += getOffset((this -> speed),(this -> rotation)).x * deltaTime * 60;
	(this -> midPos).y += getOffset((this -> speed),(this -> rotation)).y * deltaTime * 60;

	/*死亡检测*/
	if (CheckCollisionRecs((this -> rect),player.getRect())) {
		(this -> isDie) = true;
	}
	for (auto it = playerShots.begin();it != playerShots.end();it++) {
		if (CheckCollisionRecs((this -> rect),(*it) -> getRect())) {
			(this -> isDie) = true;
			(*it) -> setIsDie(true);
		}
	}

	/*获取纹理*/
	(this -> basic).setTexture(&(littlePlaneTextures.at(this -> action)));

	/*设置位置*/
	(this -> pos) = (Vector2){
		(this -> midPos).x - (float((this -> basic).getTexture() -> width) * (this -> size) / 2),
		(this -> midPos).y - (float((this -> basic).getTexture() -> height) * (this -> size) / 2)
	};

	/*设置basic*/
	(this -> basic).setSource((Rectangle){
		0,0,
		float((this -> basic).getTexture() -> width),
		float((this -> basic).getTexture() -> height)
	});
	(this -> basic).setDest((Rectangle){
		(this -> midPos).x,
		(this -> midPos).y,
		float((this -> basic).getTexture() -> width) * (this -> size),
		float((this -> basic).getTexture() -> height) * (this -> size)
	});
	(this -> basic).setOrigin((Vector2){
		float((this -> basic).getTexture() -> width) / 2 * (this -> size),
		float((this -> basic).getTexture() -> height) / 2 * (this -> size)
	});
	(this -> basic).setRotation((this -> rotation));
	(this -> basic).setColor((Color){255,255,255,255});
	(this -> basic).setYSort((this -> pos).y + ((this -> rect).height * (this -> size)));

	/*设置rect*/
	(this -> rect).width = (this -> basic).getTexture() -> width * (this -> size);
	(this -> rect).height = (this -> basic).getTexture() -> height * (this -> size);
	(this -> rect).x = (this -> pos).x;
	(this -> rect).y = (this -> pos).y;
	return;
}

bool LittlePlane::getIsDie () const {
	return (this -> isDie);
}

Rectangle LittlePlane::getRect () const {
	return (this -> rect);
}

/*函数实现*/
float lerpCorrect (float a,float b,float step,float theMinest) {
	float result = (a + ((b - a) * step));
	if (fabsf(b - result) < theMinest) {
		return b;
	}
	return result;
}

Vector2 getOffset (float step,float angle) {
	float radian = angle / 180 * (4 * atan(1));
	float offsetX = cos(radian) * step;
	float offsetY = sin(radian) * step;
	return (Vector2){offsetX,offsetY};
}

float getDeltaTime () {
	static struct timespec lastTime = {0};
	struct timespec currentTime;
	clock_gettime(CLOCK_MONOTONIC,&currentTime);
	float deltaTime = 0.0;
	if (lastTime.tv_sec != 0) {
		deltaTime = (currentTime.tv_sec - lastTime.tv_sec) + (currentTime.tv_nsec - lastTime.tv_nsec) / 1e9;
	}
	lastTime = currentTime;
	return deltaTime;
}

float getFrom (Vector2 a,Vector2 b) {
	Vector2 c = {b.x - a.x,b.y - a.y};
	return atan2f(c.y,c.x) * 180 / (4 * atan(1));
}

int randint (int a,int b) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution <> dis(a,b);
	int randomNum = dis(gen); 
	return randomNum;
}

float randfloat (float a,float b) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution <float> real_dis(a,b);
	float randomNum = real_dis(gen); 
	return randomNum;
}

float rotateLerpCorrect (float a,float b,float step,float theMinest) {
	float diff = fmodf(b - a,360.0F);
	if (diff > 180.0F) {
		diff -= 360.0F;
	}
	else if (diff < -180.0F) {
		diff += 360.0F;
	}
	float offset = diff * step;
	if (fabsf(offset) < theMinest) {
		return b;
	}
	return a + offset;
}