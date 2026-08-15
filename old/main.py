import pyxel
import time
import math
import random
lastTime = time.time()
deltaTime : float = 0
class Player(object):
	def __init__(self,tapp : App) -> None:
		self.app = tapp
		self.x : int = -100
		self.y : int = -100
		self.midX : float = 150
		self.midY : float = 200
		self.rectWidth : int = 16
		self.rectHeight : int = 16
		self.scale : float = 1
		self.speed : float = 2
		self.shotTime : float = 0
		self.rectCol : list = [0,0,16,16]
		self.hp = 6
	def update(self) -> None:
		global deltaTime
		self.x = self.midX - (self.rectWidth * self.scale / 2)
		self.y = self.midY - (self.rectHeight * self.scale / 2)
		self.rectCol[0] = self.x
		self.rectCol[1] = self.y
		self.rectCol[2] = self.rectWidth * self.scale
		self.rectCol[3] = self.rectHeight * self.scale
		keyCount : int = 0
		directionLast : float = 0.0
		isKeyUpDown : bool = pyxel.btn(pyxel.KEY_W) or pyxel.btn(pyxel.KEY_UP)
		isKeyDownDown : bool = pyxel.btn(pyxel.KEY_S) or pyxel.btn(pyxel.KEY_DOWN)
		isKeyLeftDown : bool = pyxel.btn(pyxel.KEY_A) or pyxel.btn(pyxel.KEY_LEFT)
		isKeyRightDown : bool = pyxel.btn(pyxel.KEY_D) or pyxel.btn(pyxel.KEY_RIGHT)
		if isKeyUpDown:
			directionLast += 270
			keyCount += 1
		if isKeyDownDown:
			directionLast += 90
			keyCount += 1
		if isKeyLeftDown:
			directionLast += 180
			keyCount += 1
		if isKeyRightDown:
			directionLast += 0
			keyCount += 1
		if isKeyUpDown and isKeyRightDown:
			directionLast = -45
			keyCount = 1
		if isKeyUpDown and isKeyDownDown:
			keyCount = 0
		if isKeyLeftDown and isKeyRightDown:
			keyCount = 0
		if keyCount != 0:
			self.midX += movedByDirection(self.speed * deltaTime * 60,directionLast / keyCount)[0]
			self.midY += movedByDirection(self.speed * deltaTime * 60,directionLast / keyCount)[1]
		self.shotTime += deltaTime
		isKeyShotDown : bool = pyxel.btn(pyxel.KEY_Z) or pyxel.btn(pyxel.KEY_M)
		if isKeyShotDown and (self.shotTime > 0.1):
			self.shotTime = 0
			self.app.playerShot.append(PlayerShot(self.app,self.midX,self.midY - (self.rectHeight * self.scale / 2)))
class PlayerShot(object):
	def __init__(self,tapp : App,theX : float,theY : float) -> None:
		self.app : App = tapp
		global deltaTime
		self.x : int = -100
		self.y : int = -100
		self.midX : float = theX
		self.midY : float = theY
		self.isDie : bool = False
		self.rectWidth = 8
		self.rectHeight = 8
		self.scale = 1
		self.speed = 5
		self.rectCol : list = [0,0,4,8]
	def update(self) -> None:
		self.x = self.midX - (self.rectWidth * self.scale / 2)
		self.y = self.midY - (self.rectHeight * self.scale / 2)
		self.rectCol[0] = self.x + 2
		self.rectCol[1] = self.y
		self.rectCol[2] = self.rectWidth * self.scale
		self.rectCol[3] = self.rectHeight * self.scale
		self.midY -= self.speed * deltaTime * 60
		if self.midY < -100:
			self.isDie = True
class LittlePlane(object):
	def __init__(self,tapp : App,theX : float,theY : float) -> None:
		self.app : App = tapp
		global deltaTime
		self.x : int = -100
		self.y : int = -100
		self.midX : float = theX
		self.midY : float = theY
		self.isDie : bool = False
		self.rectWidth = 16
		self.rectHeight = 16
		self.scale = 1
		self.speed = 1
		self.rectCol : list = [0,0,16,16]
		self.rotate = 90
	def update(self) -> None:
		self.x = self.midX - (self.rectWidth * self.scale / 2)
		self.y = self.midY - (self.rectHeight * self.scale / 2)
		self.rectCol[0] = self.x
		self.rectCol[1] = self.y
		self.rectCol[2] = self.rectWidth * self.scale
		self.rectCol[3] = self.rectHeight * self.scale
		self.midX += movedByDirection(
			self.speed * deltaTime * 60,
			self.rotate
		)[0]
		self.midY += movedByDirection(
			self.speed * deltaTime * 60,
			self.rotate
		)[1]
		if self.midY > self.app.screenHeight + 100:
			self.isDie = True
		for i in self.app.playerShot:
			if isTouch(self.rectCol,i.rectCol):
				self.isDie = True
				i.isDie = True
		self.rotate = rotateLerpCorrect(
			self.rotate,
			getFrom([self.midX,self.midY],[self.app.player.midX,self.app.player.midY]),
			0.1 * deltaTime * 60,
			0.1
		)
		if isTouch(self.rectCol,self.app.player.rectCol):
			self.isDie = True
			self.app.player.hp -= 1
class HpUI(object):
	def __init__(self,tapp : App) -> None:
		self.app : App = tapp
		self.x : int = 4
		self.y : int = 4
		self.hp : int = 6
		self.baseHp : int = 6
		self.rectWidth : int = 8
	def update(self) -> None:
		if self.baseHp % 2 != 0:
			self.baseHp += 1
		self.hp = self.app.player.hp
class App(object):
	def __init__(self) -> None:
		self.screenWidth : int = 300
		self.screenHeight : int = 300
		self.player = Player(self)
		self.playerShot : list = []
		self.littlePlane : list = []
		self.littlePlaneAppear : float = 0
		self.hpUI = HpUI(self)
		pyxel.init(self.screenWidth,self.screenHeight,fps = 120,title = "hello")
		pyxel.load("./assets.pyxres")
		pyxel.run(self.update,self.draw)
	def update(self) -> None:
		if pyxel.btnp(pyxel.KEY_ESCAPE):
			pyxel.quit()
		global lastTime
		global deltaTime
		currentTime = time.time()
		deltaTime = currentTime - lastTime
		lastTime = currentTime
		deltaTime = min(deltaTime,0.1)
		self.hpUI.update()
		self.player.update()
		for i in self.playerShot:
			i.update()
		self.playerShot = [i for i in self.playerShot if not i.isDie]
		for i in self.littlePlane:
			i.update()
		self.littlePlane = [i for i in self.littlePlane if not i.isDie]
		self.littlePlaneAppear += deltaTime
		if self.littlePlaneAppear > 0.01:
			self.littlePlaneAppear = 0 - (random.randint(0,100) / 100)
			self.littlePlane.append(LittlePlane(self,random.randint(0,self.screenWidth),-50))
	def draw(self) -> None:
		pyxel.cls(9)
		pyxel.blt(
			self.player.x,
			self.player.y,
			0,
			0,
			0,
			self.player.rectWidth,
			self.player.rectHeight,
			colkey = 4,
			rotate = 0,
			scale = self.player.scale
		)
		for i in self.playerShot:
			pyxel.blt(
				i.x,
				i.y,
				1,
				0,
				0,
				8,
				8,
				colkey = 4,
				rotate = 0,
				scale = 1
			)
		for i in self.littlePlane:
			pyxel.blt(
				i.x,
				i.y,
				0,
				16,
				0,
				i.rectWidth,
				i.rectHeight,
				colkey = 4,
				rotate = i.rotate,
				scale = 1
			)
		#专门画UI的位置：
		for i in range(int(self.hpUI.baseHp / 2)):
			tempPosX : int = 0
			if (2 * (i + 1)) > self.hpUI.hp:
				tempPosX = 2
			elif (2 * (i + 1)) <= self.hpUI.hp / 2:
				tempPosX = 0
			if (2 * (i + 1) - 1) == self.hpUI.hp:
				tempPosX = 1
			pyxel.blt(
				self.hpUI.x + (i * self.hpUI.rectWidth * 2),
				self.hpUI.y,
				2,
				self.hpUI.rectWidth * tempPosX,
				0,
				self.hpUI.rectWidth,
				self.hpUI.rectWidth,
				colkey = 0,
				rotate =0,
				scale = 1
			)
def movedByDirection(step,direction) -> list:
	return [
		math.cos(direction / 180 * math.pi) * step,
		math.sin(direction / 180 * math.pi) * step
	]
def isTouch(a : list,b : list) -> bool:
	return (a[0] + a[2] >= b[0]) and (a[0] <= b[0] + b[2]) and (a[1] + a[3] >= b[1]) and (a[1] <= b[1] + b[3])
def getFrom(pos1 : list,pos2 : list) -> float:
	offset : list = [
			pos2[0] - pos1[0],
			pos2[1] - pos1[1]
	]
	return math.atan2(offset[1],offset[0]) / math.pi * 180
def lerpCorrect(a1 : float,a2 : float,step : float,smallest : float) -> float:
	offset : float = (a2 - a1) * step
	if abs(offset) < smallest:
		return a2
	return a1 + offset
def rotateLerpCorrect(a1 : float,a2 :float,step : float,smallest : float) -> float:
	diff = (a2 - a1) % 360
	if diff > 180:
		diff -= 360
	elif diff < -180:
		diff += 360
	offset = diff * step
	if abs(offset) < smallest:
		return a2
	return a1 + offset;
if __name__ == '__main__':
	app = App()
