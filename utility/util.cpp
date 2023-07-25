#include "util.h"
#include "DirectX.h"

bool util::CreateHook(uint16_t Index, void** Original, void* Function) {
	assert(_index >= 0 && _original != NULL && _function != NULL);
	void* target = (void*)DirectXHelper::MethodsTable[Index];
	if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK) {
		return false;
	}
	return true;
}

void util::DisableHook(uint16_t Index) {
	assert(Index >= 0);
	MH_DisableHook((void*)DirectXHelper::MethodsTable[Index]);
}

void util::DisableAll() {
	MH_DisableHook(MH_ALL_HOOKS);
	free(DirectXHelper::MethodsTable);
	DirectXHelper::MethodsTable = NULL;
}
void util::createDebugConsole() {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
}

bool isPtrInvalid(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	if (::VirtualQuery(p, &mbi, sizeof(mbi)))
	{
		DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
		bool b = !(mbi.Protect & mask);
		// check the page is not a guard page
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

		return b;
	}
	return true;
}

uintptr_t util::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	if (addr == 0) { return 0; }

	for (unsigned int i = 0; i < offsets.size(); ++i) {
		// Check if the current address is valid
		if (isPtrInvalid((void*)addr)) {
			return 0;
		}

		addr = *(uintptr_t*)addr;
		if (addr == 0) { return 0; }
		addr += offsets[i];
	}
	return addr;
}

glm::dmat3 util::createRotationMatrix(double yaw, double pitch) {
	glm::dmat3 rotationMatrix;

	double yawRadians = glm::radians(yaw);
	double pitchRadians = glm::radians(pitch);

	rotationMatrix[0][0] = cos(yawRadians);
	rotationMatrix[0][1] = 0;
	rotationMatrix[0][2] = sin(yawRadians);

	rotationMatrix[1][0] = sin(pitchRadians) * sin(yawRadians);
	rotationMatrix[1][1] = cos(pitchRadians);
	rotationMatrix[1][2] = -sin(pitchRadians) * cos(yawRadians);

	rotationMatrix[2][0] = -cos(pitchRadians) * sin(yawRadians);
	rotationMatrix[2][1] = sin(pitchRadians);
	rotationMatrix[2][2] = cos(pitchRadians) * cos(yawRadians);

	return rotationMatrix;
}

void util::printMatrix(float matrix[16])
{
	for (int i = 0; i < 16; i+=4) {
		for (int j = 0; j < 4; j++) {
			std::cout << matrix[i+j] << " | ";
		}
		std::cout << std::endl;
	}
}

glm::dvec3 util::calculateRotations(double yaw, double pitch) {
	glm::dmat3 rotationMatrix = createRotationMatrix(yaw, pitch);

	double xRotation = atan2(rotationMatrix[2][1], rotationMatrix[2][2]);
	double yRotation = atan2(-rotationMatrix[2][0], sqrt(rotationMatrix[2][1] * rotationMatrix[2][1] + rotationMatrix[2][2] * rotationMatrix[2][2]));
	double zRotation = atan2(rotationMatrix[1][0], rotationMatrix[0][0]);

	return glm::dvec3(glm::degrees(xRotation), glm::degrees(yRotation), glm::degrees(zRotation));
}

DirectX::XMFLOAT2 util::WorldToScreen(const DirectX::XMVECTOR& worldPos, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, const UINT viewportWidth, const UINT viewportHeight)
{
	// Convert world position to clip space
	DirectX::XMVECTOR clipSpacePos = DirectX::XMVector3TransformCoord(worldPos, viewMatrix);
	clipSpacePos = DirectX::XMVector3TransformCoord(clipSpacePos, projectionMatrix);

	// Convert clip space to NDC space
	DirectX::XMFLOAT4 clipSpacePosFloat;
	DirectX::XMStoreFloat4(&clipSpacePosFloat, clipSpacePos);

	DirectX::XMFLOAT3 ndcSpacePos = DirectX::XMFLOAT3(clipSpacePosFloat.x / clipSpacePosFloat.w, clipSpacePosFloat.y / clipSpacePosFloat.w, clipSpacePosFloat.z / clipSpacePosFloat.w);

	// Convert NDC space to screen space
	float screenX = (1.0f - ndcSpacePos.x) * 0.5f * static_cast<float>(viewportWidth);
	float screenY = (1.0f - ndcSpacePos.y) * 0.5f * static_cast<float>(viewportHeight);

	return DirectX::XMFLOAT2(screenX, screenY);
}

float util::convertHorizontalToVerticalFOV(float horizontalFOV, float aspectRatio)
{
	float verticalFOV = 2.0f * atanf(tanf(horizontalFOV * 0.5f) / aspectRatio);
	return verticalFOV;
}

Vec3 util::vectorSubtract(Vec3 vec1, Vec3 vec2) {
	Vec3 diff{0,0,0};
	diff.x = vec1.x - vec2.x;
	diff.y = vec1.y - vec2.y;
	diff.z = vec1.z - vec2.z;
	return diff;
}

float util::degreesToRadians(float degrees) {
	return degrees * (3.14159265358979323846f / 180);
}

Vec3 util::crossProduct(const Vec3 a, const Vec3 b) {
	return Vec3(
		a.x * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

Vec3 util::getSafeNormal(const Vec3 vec, float tolerance)
{
	float lengthSquared = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
	if (lengthSquared > tolerance)
	{
		float length = std::sqrt(lengthSquared);
		return Vec3(vec.x / length, vec.y / length, vec.z / length);
	}
	else
	{
		return Vec3(0.0f, 0.0f, 0.0f);
	}
}

float util::dotProduct(const Vec3 vec1, const Vec3 vec2) {
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}