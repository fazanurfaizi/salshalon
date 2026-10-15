#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "features/salon/SalonContext.hpp"
#include "features/salon/customers/Customer.hpp"
#include "math/Math.hpp"

namespace salshalon::features::salon {

struct CustomerEvents {
  /// No free wait slot when a spawn was due (state shows a warning toast).
  bool spawnRejected = false;
  /// Successful spawns (state plays the doorbell).
  int spawned = 0;
  /// Service completions at station positions (hearts + dust).
  std::vector<math::Vec2> completedAt;
  /// Payments at customer positions (coins + pulse + chime + tutorial).
  std::vector<math::Vec2> servedAt;
  /// Customers lost (shake + red pulse).
  int gaveUp = 0;

  void clear();
};

enum class SeatResult {
  Seated,     // outSeatPos holds the station center (dust + tap)
  NoCustomer, // nothing tappable there - silent
  NoStation,  // hit, but no free station (state shows a warning toast)
};

class CustomerManager {
public:
  static constexpr float kCustomerHitRadius = 48.0f;
  static constexpr float kFirstSpawnDelaySeconds = 2.0f;

  void reset();

  void update(float deltaSeconds, SalonContext &context,
              CustomerEvents &events);

  SeatResult trySeatAt(math::Vec2 worldPos, SalonContext &context,
                       math::Vec2 &outSeatPos);

  const std::vector<Customer> &customers() const { return customers_; }
  uint32_t served() const { return sessionServed_; }
  uint32_t lost() const { return sessionLost_; }
  float elapsed() const { return elapsed_; }

private:
  void spawnTick(float deltaSeconds, SalonContext &context,
                 CustomerEvents &events);
  void spawnCustomer(SalonContext &context, CustomerEvents &events);
  void applyCustomerTransition(Customer &customer, SalonContext &context,
                               CustomerEvents &events);
  void removeGoneCustomers(SalonContext &context, CustomerEvents &events);

  int findFreeWaitSlot(const SalonContext &context) const;
  int findFreeStation(const SalonContext &context) const;
  void reserveCell(SalonContext &context, int index, uint32_t customerId);
  void freeCell(SalonContext &context, int index);
  void freeCustomerCell(SalonContext &context, Customer &customer);

  std::vector<Customer> customers_;
  uint32_t nextCustomerId_ = 0;

  float elapsed_ = 0.0f;
  float timeUntilNextSpawn_ = kFirstSpawnDelaySeconds;

  uint32_t sessionServed_ = 0;
  uint32_t sessionLost_ = 0;

  std::mt19937 rng_{0x5A15A10Fu};
};

} // namespace salshalon::features::salon
