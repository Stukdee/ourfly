#include "raylib.h"
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <algorithm>

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

/*纹理*/
std::vector <Texture2D> playerTextures;
std::vector <Texture2D> playerShotTextures;

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
};

/*实例化类*/
std::vector <BasicInfo*> basicInfos;
Player player;
std::vector <PlayerShot*> playerShots;

/*函数声明与定义*/
static bool compareByYSort(const BasicInfo *a,const BasicInfo *b){
	return a -> getYSort() < b -> getYSort();
}

/*函数声明*/
float lerpCorrect (float a,float b,float step,float theMinest);

Vector2 getOffset (float step,float angle);

float getDeltaTime ();


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

	/*导入素材*/
	playerTextures.push_back(LoadTexture((path + std::string("/assets/player/1.png")).c_str()));
	playerShotTextures.push_back(LoadTexture((path + std::string("/assets/playerShot/1.png")).c_str()));

	while (!WindowShouldClose()) {
		/*更新*/
		deltaTime = getDeltaTime();
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

		/*开始绘制*/
		BeginDrawing();
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
		EndDrawing();
	}
	/*释放资源*/
	for (auto it = playerTextures.begin();it != playerTextures.end();it++) {
		UnloadTexture(*it);
	}
	for (auto it = playerShotTextures.begin();it != playerShotTextures.end();it++) {
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

/*Player类的实现*/
Player::Player () {
	(this -> midPos) = (Vector2){SCREEN_WIDTH / 2,SCREEN_HEIGHT - (SCREEN_HEIGHT / 4)};
	(this -> action) = 0;
	(this -> speed) = 0.5;
	(this -> size) = 2.0F;
}

void Player::update () {
	/*静态变量*/
	static float shotInterval = 0.0F;

	/*更新间隔*/
	shotInterval += deltaTime;

	/*玩家移动*/
	int stepCount = 0;
	float angle = 0;
	if(KEY_CONTROL_UP){angle += 270;stepCount++;}
	if(KEY_CONTROL_DOWN){angle += 90;stepCount++;}
	if(KEY_CONTROL_LEFT){angle += 180;stepCount++;}
	if(KEY_CONTROL_RIGHT){angle += 0;stepCount++;}
	if(KEY_CONTROL_UP && KEY_CONTROL_RIGHT){angle = 315;stepCount = 1;}
	if(KEY_CONTROL_UP && KEY_CONTROL_DOWN){stepCount = 0;}
	if(KEY_CONTROL_RIGHT && KEY_CONTROL_LEFT){stepCount = 0;}
	if(stepCount != 0){
		(this -> midPos).x += getOffset((this -> speed),angle / stepCount).x;
		(this -> midPos).y += getOffset((this -> speed),angle / stepCount).y;
	}
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

void PlayerShot::update () {
	/*移动*/
	(this -> midPos).y -= (this -> speed) * deltaTime * 60;
	if ((this -> midPos).y < -1000) {
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

/*函数实现*/
float lerpCorrect (float a,float b,float step,float theMinest) {
	float result = (a + ((b - a) * step));
	if (fabsf(b - result) < theMinest) {
		return b;
	}
	return result;
}

Vector2 getOffset (float step,float angle) {
	float radian = angle / 180 * M_PI;
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