#include "uuid.hpp"
#include <random>
#include <unordered_set>

using namespace CitronCore;

uint32_t UUID::nullID = 0;

static std::random_device randomDevice;
static std::mt19937 randomEngine(randomDevice());
static std::uniform_int_distribution<uint32_t>
	uniformDistribution(0, std::numeric_limits<uint32_t>::max());

UUID::UUID() : m_uuid(uniformDistribution(randomEngine)) {}

UUID::UUID(uint32_t uuid) : m_uuid(uuid) {}
