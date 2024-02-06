#pragma once
#include <vector>

struct GLTFsamplerInput
{
	int accessorId = -1;
	float* input = NULL;
	int inputsN = 0;
	//prev and next indices
	int iPrev = -1;
	int iNext = -1;
	float timePrev = -1;
	float timeNext = -1;
	float timeRate = 1;
};
struct GLTFsamplerOutput
{
	int accessorId = -1;
	float* output = NULL;
	int outputsN = 0;
	int strideInFloats = 0;
};
struct GLTFsampler
{
	char interpolation[32] = "";
	int inputN = -1;
	int outputN = -1;
};
struct GLTFchannel
{
	int samplerN;              // required
	int target_node;          // optional index of the node to target (alternative target should be provided by extension)
	char target_path[32] = "";  // required with standard values of ["translation",
};

class GLTFanimation
{
public:
	char animName[32] = "";
	std::vector<GLTFsampler*> samplers;
	std::vector<GLTFchannel*> channels;

	std::vector<GLTFsamplerInput*> inputs;
	std::vector<GLTFsamplerOutput*> outputs;

	float timeTotal = 0;

	int animSkeletonMapN = -1;

	static std::vector<GLTFanimation*> GLTFanimations_default;

};

