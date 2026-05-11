#include <iostream>
#include <vector>
#include <random>

using namespace std;

struct Particle
{
	vector<float> position = vector<float>(3);
};

vector<float> operator-(vector<float> & x, vector<float> & y)
{
	vector<float> ret(3);
	for (size_t i = 0; i < 3; i++)
	{
		ret[i] = x[i] - y[i];
	}
	return ret;
}

int main()
{
	random_device rd;
	mt19937 gen(rd());
	float radius = 0.01f;
	float spacing = radius * 2.f;
	float h = spacing;

	float minX = -0.5f;
	float minY = -0.5f;
	float minZ = -0.5f;
	float maxX = 0.5f;
	float maxY = 0.5f;
	float maxZ = 0.5f;

	float xLen = maxX - minX;
	float yLen = maxY - minY;
	float zLen = maxZ - minZ;

	int DimX = floor(xLen / h);
	int DimY = floor(yLen / h);
	int DimZ = floor(zLen / h);

	vector<float> basePos = { minX, minY, minZ };
	std::uniform_real_distribution<float> fd(minX + radius, maxX - radius);
	int particleCount = 1000;

	vector<Particle> particles(particleCount);
	vector<Particle> sortedParticles(particleCount);
	vector<int> particleCounter(DimX*DimY*DimZ, 0);
	vector<int> cellStart(DimX*DimY*DimZ, 0);
	vector<int> cellStart_copy(DimX*DimY*DimZ, 0);

	for (size_t i = 0; i < particleCount; i++)
	{
		particles[i].position = { fd(gen), fd(gen), fd(gen) };
	}

	// pass 1
	for (size_t Dtid = 0; Dtid < particleCount; Dtid++)
	{
		auto pos = particles[Dtid].position;

		auto diff = (pos - basePos);
		int xId = floor(DimX * diff[0] / xLen);
		int yId = floor(DimY * diff[1] / yLen);
		int zId = floor(DimZ * diff[2] / zLen);

		int id = xId + yId * DimX + zId * DimX * DimY;
		particleCounter[id]++;
	}

    // pass 2
    for (size_t cellId = 1; cellId < DimX*DimY*DimZ; cellId++)
	{
		cellStart[cellId] = cellStart[cellId-1] + particleCounter[cellId-1];
		
	}

    // pass 3
    for (size_t Dtid = 0; Dtid < particleCount; Dtid++)
	{
		auto pos = particles[Dtid].position;
		auto diff = (pos - basePos);
		int xId = floor(DimX * diff[0] / xLen);
		int yId = floor(DimY * diff[1] / yLen);
		int zId = floor(DimZ * diff[2] / zLen);

		int cellId = xId + yId * DimX + zId * DimX * DimY;
        int startIdx = cellStart_copy[cellId]++;
		sortedParticles[startIdx] = particles[Dtid];
	}
}
