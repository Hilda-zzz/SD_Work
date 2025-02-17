#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"

Mat44::Mat44()
{
	for (int i = 0; i < 16; i++)
	{
		if (i == Ix || i == Jy || i == Kz || i == Tw)
		{
			m_values[i] = 1.f;
		}
		else
			m_values[i] = 0.f;
	}
}

Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	for (int i = 0; i < 16; i++)
	{
		if (i == Ix || i == Jy || i == Kz || i == Tw)
		{
			m_values[i] = 1.f;
		}
		else
			m_values[i] = 0.f;
	}
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
}

Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	for (int i = 0; i < 16; i++)
	{
		if (i == Ix || i == Jy || i == Kz || i == Tw)
		{
			m_values[i] = 1.f;
		}
		else
			m_values[i] = 0.f;
	}
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;

}

Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

Mat44::Mat44(float const* sixteenValueBasisMajor)
{
	for (int i = 0; i < 16; i++)
	{
		m_values[i] = sixteenValueBasisMajor[i];
	}
}

Mat44 const Mat44::MakeTranslation2D(Vec2 const& translationXY)
{
	Mat44 mat=Mat44();
	mat.m_values[Tx] = translationXY.x;
	mat.m_values[Ty] = translationXY.y;
	return mat;
}

Mat44 const Mat44::MakeTranslation3D(Vec3 const& translationXY)
{
	Mat44 mat = Mat44();
	mat.m_values[Tx] = translationXY.x;
	mat.m_values[Ty] = translationXY.y;
	mat.m_values[Tz] = translationXY.z;
	return mat;
}

Mat44 const Mat44::MakeUniformScale2D(float uniformScaleXY)
{
	Mat44 mat = Mat44();
	mat.m_values[Ix] = uniformScaleXY;
	mat.m_values[Jy] = uniformScaleXY;
	return mat;
}

Mat44 const Mat44::MakeUniformScale3D(float uniformScaleXYZ)
{
	Mat44 mat = Mat44();
	mat.m_values[Ix] = uniformScaleXYZ;
	mat.m_values[Jy] = uniformScaleXYZ;
	mat.m_values[Kz] = uniformScaleXYZ;
	return mat;
}

Mat44 const Mat44::MakeNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 mat = Mat44();
	mat.m_values[Ix] = nonUniformScaleXY.x;
	mat.m_values[Jy] = nonUniformScaleXY.y;
	return mat;
}

Mat44 const Mat44::MakeNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 mat = Mat44();
	mat.m_values[Ix] = nonUniformScaleXYZ.x;
	mat.m_values[Jy] = nonUniformScaleXYZ.y;
	mat.m_values[Kz] = nonUniformScaleXYZ.z;
	return mat;
}

Mat44 const Mat44::MakeZRotationDegrees(float rotationDegreesAboutZ)
{
	Mat44 mat = Mat44();
	mat.m_values[Ix] = CosDegrees(rotationDegreesAboutZ);
	mat.m_values[Iy] = SinDegrees(rotationDegreesAboutZ);
	mat.m_values[Jx] = -SinDegrees(rotationDegreesAboutZ);
	mat.m_values[Jy] = CosDegrees(rotationDegreesAboutZ);
	return mat;
}

Mat44 const Mat44::MakeYRotationDegrees(float rotationDegreesAboutY)
{
	Mat44 mat = Mat44();
	mat.m_values[Ix] = CosDegrees(rotationDegreesAboutY);
	mat.m_values[Iz] = -SinDegrees(rotationDegreesAboutY);
	mat.m_values[Kx] = SinDegrees(rotationDegreesAboutY);
	mat.m_values[Kz] = CosDegrees(rotationDegreesAboutY);
	return mat;
}

Mat44 const Mat44::MakeXRotationDegrees(float rotationDegreesAboutX)
{
	Mat44 mat = Mat44();
	mat.m_values[Jy] = CosDegrees(rotationDegreesAboutX);
	mat.m_values[Jz] = SinDegrees(rotationDegreesAboutX);
	mat.m_values[Ky] = -SinDegrees(rotationDegreesAboutX);
	mat.m_values[Kz] = CosDegrees(rotationDegreesAboutX);
	return mat;
}

Mat44 const Mat44::MakeOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Mat44 mat = Mat44();
	mat.m_values[Ix] = 2.f / (right - left);
	mat.m_values[Jy] = 2.f / (top - bottom);
	mat.m_values[Kz] = 1.f / (zFar - zNear);

	mat.m_values[Tx] = -(right + left) / (right - left);
	mat.m_values[Ty] = -(top + bottom) / (top - bottom);
	mat.m_values[Tz] = -zNear / (zFar - zNear);

	return mat;
}

Mat44 const Mat44::MakePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat44 mat = Mat44();
	float Sh = 2.0f * (SinDegrees(fovYDegrees * 0.5f) / CosDegrees(fovYDegrees * 0.5f));
	float Sw = Sh * aspect;

	mat.m_values[Ix] = 2.0f / Sw;
	mat.m_values[Jy] = 2.0f / Sh;
	mat.m_values[Kz] = zFar / (zFar - zNear);
	mat.m_values[Tz] = -zFar * zNear / (zFar - zNear);
	mat.m_values[Kw] = 1.0f;
	mat.m_values[Tw] = 0.0f;

	return mat;
}

Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const  //w = 0
{
	float Px = m_values[Ix] * vectorQuantityXY.x + m_values[Jx] * vectorQuantityXY.y;
	float Py= m_values[Iy] * vectorQuantityXY.x + m_values[Jy] * vectorQuantityXY.y;
	return Vec2(Px, Py);
}

Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	float Px = m_values[Ix] * vectorQuantityXYZ.x + m_values[Jx] * vectorQuantityXYZ.y + m_values[Kx] * vectorQuantityXYZ.z;
	float Py = m_values[Iy] * vectorQuantityXYZ.x + m_values[Jy] * vectorQuantityXYZ.y + m_values[Ky] * vectorQuantityXYZ.z;
	float Pz = m_values[Iz] * vectorQuantityXYZ.x + m_values[Jz] * vectorQuantityXYZ.y + m_values[Kz] * vectorQuantityXYZ.z;
	return Vec3(Px, Py, Pz);
}

Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	float Px = m_values[Ix] * positionXY.x + m_values[Jx] * positionXY.y + m_values[Tx];
	float Py = m_values[Iy] * positionXY.x + m_values[Jy] * positionXY.y + m_values[Ty];
	return Vec2(Px, Py);
}

Vec3 const Mat44::TransformPosition3D(Vec3 const& position3D) const
{
	float Px = m_values[Ix] * position3D.x + m_values[Jx] * position3D.y + m_values[Kx] * position3D.z + m_values[Tx];
	float Py = m_values[Iy] * position3D.x + m_values[Jy] * position3D.y + m_values[Ky] * position3D.z + m_values[Ty];
	float Pz = m_values[Iz] * position3D.x + m_values[Jz] * position3D.y + m_values[Kz] * position3D.z + m_values[Tz];
	return Vec3(Px, Py, Pz);								
}

Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	float Px = m_values[Ix] * homogeneousPoint3D.x + m_values[Jx] * homogeneousPoint3D.y + m_values[Kx] * homogeneousPoint3D.z + m_values[Tx] * homogeneousPoint3D.w;
	float Py = m_values[Iy] * homogeneousPoint3D.x + m_values[Jy] * homogeneousPoint3D.y + m_values[Ky] * homogeneousPoint3D.z + m_values[Ty] * homogeneousPoint3D.w;
	float Pz = m_values[Iz] * homogeneousPoint3D.x + m_values[Jz] * homogeneousPoint3D.y + m_values[Kz] * homogeneousPoint3D.z + m_values[Tz] * homogeneousPoint3D.w;
	float Pw = m_values[Iw] * homogeneousPoint3D.x + m_values[Jw] * homogeneousPoint3D.y + m_values[Kw] * homogeneousPoint3D.z + m_values[Tw] * homogeneousPoint3D.w;
	return Vec4(Px, Py, Pz, Pw);
}

float* Mat44::GetAsFloatArray()
{
	return m_values;
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy],m_values[Iz]);
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 inverse;
	inverse.m_values[Ix] = m_values[Ix];
	inverse.m_values[Iy] = m_values[Jx];
	inverse.m_values[Iz] = m_values[Kx];

	inverse.m_values[Jx] = m_values[Iy];
	inverse.m_values[Jy] = m_values[Jy];
	inverse.m_values[Jz] = m_values[Ky];

	inverse.m_values[Kx] = m_values[Iz];
	inverse.m_values[Ky] = m_values[Jz];
	inverse.m_values[Kz] = m_values[Kz];
	
	inverse.m_values[Tx] = -(m_values[Ix] * m_values[Tx] +
		m_values[Iy] * m_values[Ty] +
		m_values[Iz] * m_values[Tz]);

	inverse.m_values[Ty] = -(m_values[Jx] * m_values[Tx] +
		m_values[Jy] * m_values[Ty] +
		m_values[Jz] * m_values[Tz]);

	inverse.m_values[Tz] = -(m_values[Kx] * m_values[Tx] +
		m_values[Ky] * m_values[Ty] +
		m_values[Kz] * m_values[Tz]);

	inverse.m_values[Iw] = 0.f;
	inverse.m_values[Jw] = 0.f;
	inverse.m_values[Kw] = 0.f;
	inverse.m_values[Tw] = 1.0f;
	return inverse;
}

void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;
}

void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;
}

void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

void Mat44::Transpose()
{
	Mat44 transpose;

	transpose.m_values[Ix] = m_values[Ix];    
	transpose.m_values[Iy] = m_values[Jx];    
	transpose.m_values[Iz] = m_values[Kx];    
	transpose.m_values[Iw] = m_values[Tx];    

	transpose.m_values[Jx] = m_values[Iy];    
	transpose.m_values[Jy] = m_values[Jy];    
	transpose.m_values[Jz] = m_values[Ky];    
	transpose.m_values[Jw] = m_values[Ty];    

	transpose.m_values[Kx] = m_values[Iz];    
	transpose.m_values[Ky] = m_values[Jz];    
	transpose.m_values[Kz] = m_values[Kz];    
	transpose.m_values[Kw] = m_values[Tz];    

	transpose.m_values[Tx] = m_values[Iw];    
	transpose.m_values[Ty] = m_values[Jw];    
	transpose.m_values[Tz] = m_values[Kw];    
	transpose.m_values[Tw] = m_values[Tw];    

	for (int i = 0; i < 15; i++)
	{
		m_values[i] = transpose.m_values[i];
	}
}

void Mat44::Orthonormalize_XFwd_YLeft_ZUp()
{
	Vec3 i= Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
	i.Normalized();
	Vec3 k = Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
	Vec3 ki = i * DotProduct3D(i, k);
	k -= ki;
	k.Normalized();

	Vec3 j = Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
	Vec3 ji= i * DotProduct3D(j, i);
	Vec3 jk = k * DotProduct3D(j, k);
	j -= ji; j -= jk;
	j.Normalized();

	SetIJK3D(i, j, k);
	
}

void Mat44::Append(Mat44 const& appendThis)
{
	Mat44 resultMat = Mat44();
	//change to hard code to speed up 
	// float constsS* left=&copyOfThis.m_values[0]
	for (int i = 0; i < 4; ++i) 
	{
		for (int j = 0; j < 4; ++j) 
		{
			resultMat.m_values[i * 4 + j] =
				appendThis.m_values[i * 4] * m_values[j] +
				appendThis.m_values[i * 4 + 1] * m_values[j + 4] +
				appendThis.m_values[i * 4 + 2] * m_values[j + 8] +
				appendThis.m_values[i * 4 + 3] * m_values[j + 12];
		}
	}

	for (int i = 0; i < 16; i++) 
	{
		m_values[i] = resultMat.m_values[i];
	}
}

void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	//calculate by math and come out a formula will be more optimize
	//#to do : rewrite
	Append(MakeZRotationDegrees(degreesRotationAboutZ));
}

void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Append(MakeYRotationDegrees(degreesRotationAboutY));
}

void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Append(MakeXRotationDegrees(degreesRotationAboutX));
}

void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Append(MakeTranslation2D(translationXY));
}

void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Append(MakeTranslation3D(translationXYZ));
}

void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Append(MakeUniformScale2D(uniformScaleXY));
}

void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Append(MakeUniformScale3D(uniformScaleXYZ));
}

void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Append(MakeNonUniformScale2D(nonUniformScaleXY));
}

void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUNiformSccaleXYZ)
{
	Append(MakeNonUniformScale3D(nonUNiformSccaleXYZ));
}
