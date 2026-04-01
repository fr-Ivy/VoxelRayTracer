#pragma once
class Material
{
public:
	virtual float3 Calc(Renderer& renderer, const Ray& ray, const float3 N, float3 I, const int depth) const = 0;
private:

};


//pseudo 

//
//struct materials
//{
//	vector<Material> mats; mats[i].calc();
//	vector<Dialectics> dialectricsMaterials;
//	//hybrid
//	//reflective
//};