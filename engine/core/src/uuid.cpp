#include "uuid.hpp"
#include <random>
#include <unordered_set>

using namespace CitronCore;

static std::random_device randomDevice;
static std::mt19937 randomEngine(randomDevice());
static std::uniform_int_distribution<uint64_t>
	uniformDistribution(0, std::numeric_limits<uint64_t>::max());

UUID::UUID() : m_uuid(uniformDistribution(randomEngine)) {}

UUID::UUID(uint64_t uuid) : m_uuid(uuid) {}
