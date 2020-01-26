#pragma once


class YCamera
{
public:

	typedef enum {
		PROJECTION_ORTHO,
		PROJECTION_PERSPECTIVE
	} PROJTYPE;

	
	YVec3<float> Position; ///< Position de la camera
	YVec3<float> LookAt; ///< Point regarde par la camera
	YVec3<float> Direction; ///< Direction de la camera
	YVec3<float> UpVec; ///< Vecteur up de la camera
	YVec3<float> RightVec; ///< Si on se place dans la camera, indique la droite	
	YVec3<float> UpRef; ///< Ce qu'on considère comme le "haut" dans notre monde (et pas le up de la cam)
	

	PROJTYPE ProjType; ///< Type de projection
	float FovY; ///< fov angle sur y in degrees
	float Ratio; ///< with / height
	float Left; ///< Left YPlane
	float Right;
	float Bottom;
	float Top;
	float Near;
	float Far;
			
	YCamera()
	{
		Position = YVec3<float>(0, -1, 0);
		LookAt = YVec3<float>(0, 0, 0);
		UpRef = YVec3<float>(0, 0, 1);
		UpVec = UpRef;
		FovY = 45.0f;
		Ratio = 800.0f / 600.0f;
		Near = 0.1f;
		Far = 800.0f;
		setProjectionPerspective(FovY, Ratio, Near, Far);
		updateVecs();
	}

	void setProjectionPerspective(float fovy, float ratio, float nearPlane, float farPlane) {
		ProjType = PROJECTION_PERSPECTIVE;
		FovY = fovy;
		Ratio = ratio;
		Near = nearPlane;
		Far = farPlane;
	}

	void setProjectionOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
		ProjType = PROJECTION_ORTHO;
		Left = left;
		Right = right;
		Bottom = bottom;
		Top = top;
		Near = nearPlane;
		Far = farPlane;
	}


	/**
	* Mise a jour de la camera
	*/
	virtual void update(float elapsed)
	{

	}

	/**
	* Definition du point regarde
	*/
	void setLookAt(YVec3<float> lookat)
	{
		LookAt = lookat;
		updateVecs();
	}

	/**
	* Definition de la position de la camera
	*/
	void setPosition(YVec3<float> pos)
	{
		Position = pos;
		updateVecs();
	}

	/**
	* Definition du haut de notre monde
	*/
	void setUpRef(YVec3<float> upRef)
	{
		UpRef = upRef;
		updateVecs();
	}

	/**
	* Deplacement de la camera d'un delta donné
	*/
	void move(YVec3<float> delta)
	{
		Position += delta;
		LookAt = Position + Direction;
	}
	void relativeMove(YVec3<float> delta) {
		move(Direction * delta.X + RightVec * delta.Y + UpVec * delta.Z);
	}
	void moveWorld(YVec3<float> delta) {
		move((Direction * YVec3f(1, 1, 0)).normalize() * delta.X + (RightVec * YVec3f(1, 1, 0)).normalize() * delta.Y);
	}
	/**
	* Deplacement de la camera à un point donné
	*/
	void moveTo(const YVec3<float> & target)
	{
		Position = target;
		LookAt = Position + Direction;
	}


	/**
	* On recalcule les vecteurs utiles au déplacement de la camera (Direction, RightVec, UpVec)
	* on part du principe que sont connus :
	* - la position de la camera
	* - le point regarde par la camera
	* - la vecteur up de notre monde
	*/
	void updateVecs(void)
	{
		Direction = (LookAt - Position).normalize();
		RightVec = UpRef.cross(Direction).normalize();
		UpVec = Direction.cross(RightVec).normalize();
		
	}
	bool updateVecsUp(void) {
		Direction = (LookAt - Position).normalize();
		RightVec = UpRef.cross(Direction);
		if (RightVec.getSize() < 0.1f) {
			return false;
		}
		RightVec= RightVec.normalize();
		UpVec = Direction.cross(RightVec).normalize();
		return true;
	}
	/**
	* Rotation droite gauche en subjectif
	*/
	void rotate(float angle)
	{
		Direction.rotate(-UpVec,angle);
		LookAt = Position + Direction;
		updateVecs();
	}

	/**
	* Rotation haut bas en subjectif
	*/
	void rotateUp(float angle)
	{
		YVec3f last = Direction;
		Direction.rotate(RightVec, angle);
		LookAt = Position + Direction;
		if (!updateVecsUp()) {
			Direction = last;
			LookAt = Position + Direction;
			updateVecsUp();
		}

	}

	/**
	* Rotation droite gauche en troisième personne
	*/
	void rotateAround(float angle)
	{
		Direction.rotate(UpVec, -angle);
		Position = LookAt - Direction;
		updateVecs();
	}

	/**
	* Rotation haut bas en troisième personne
	*/
	void rotateUpAround(float angle)
	{
		YVec3f last = Direction;
		Direction.rotate(RightVec, -angle);
		Position = LookAt - Direction;
		if (!updateVecsUp()) {
			Direction = last;
			Position = LookAt - Direction;
			updateVecsUp();
		}
	}

	/**
	* Calcul du bon repère de départ pour la matrice monde
	*/
	void look(void)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (ProjType == PROJECTION_PERSPECTIVE) {
			gluPerspective(FovY, Ratio, Near, Far);
		}
		if (ProjType == PROJECTION_ORTHO) {
			glOrtho(Left, Right, Bottom, Top, Near, Far);
		}
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(Position.X, Position.Y, Position.Z, LookAt.X, LookAt.Y, LookAt.Z, UpVec.X, UpVec.Y, UpVec.Z);
		
	}
};
