#include "features/salon/play/CustomerManager.hpp"

#include <glm/geometric.hpp>
#include <random>

#include "core/Log.hpp"
#include "features/salon/customers/CustomerCatalog.hpp"
#include "features/salon/DifficultyCurve.hpp"
#include "features/salon/SalonLayout.hpp"

namespace salshalon::features::salon {

void CustomerEvents::clear() {
  spawnRejected = false;
  spawned = 0;
  completedAt.clear();
  servedAt.clear();
  gaveUp = 0;
}

void CustomerManager::reset() {
  customers_.clear();
  nextCustomerId_ = 0;
  elapsed_ = 0.0f;
  timeUntilNextSpawn_ = kFirstSpawnDelaySeconds;
  sessionServed_ = 0;
  sessionLost_ = 0;
  rng_.seed(0x5A15A10Fu);
}

void CustomerManager::update(float deltaSeconds, SalonContext &context,
                             CustomerEvents &events) {
  events.clear();
  elapsed_ += deltaSeconds;

  spawnTick(deltaSeconds, context, events);

  for (auto &customer : customers_) {
    customer.update(deltaSeconds);
  }

  for (auto &customer : customers_) {
    applyCustomerTransition(customer, context, events);
  }

  removeGoneCustomers(context, events);
}

SeatResult CustomerManager::trySeatAt(math::Vec2 worldPos,
                                      SalonContext &context,
                                      math::Vec2 &outSeatPos) {
  Customer *hit = nullptr;
  float bestDist = kCustomerHitRadius;

  for (auto &customer : customers_) {
    if (customer.currentState() != CustomerState::Queueing)
      continue;

    const float d = glm::distance(customer.currentPosition(), worldPos);
    if (d < bestDist) {
      bestDist = d;
      hit = &customer;
    }
  }
  if (hit == nullptr)
    return SeatResult::NoCustomer;

  const int stationIndex = findFreeStation(context);
  if (stationIndex < 0) {
    SAL_LOGW("Salon: no free chair for customer: %u", hit->id());
    return SeatResult::NoStation;
  }

  freeCustomerCell(context, *hit);
  reserveCell(context, stationIndex, hit->id());

  hit->setReserveCellIndex(stationIndex);
  outSeatPos = SalonLayout::cellCenterByIndex(stationIndex);
  hit->setTargetPosition(outSeatPos);
  hit->setCurrentState(CustomerState::WalkingToSeat);

  SAL_LOGI("Salon: customer %u -> station %d", hit->id(), stationIndex);
  return SeatResult::Seated;
}

void CustomerManager::spawnTick(float deltaSeconds, SalonContext &context,
                                CustomerEvents &events) {
  timeUntilNextSpawn_ -= deltaSeconds;
  if (timeUntilNextSpawn_ > 0.0f)
    return;

  const std::size_t before = customers_.size();
  spawnCustomer(context, events);
  if (customers_.size() == before)
    return;

  std::uniform_real_distribution<float> unit(0.0f, 1.0f);
  timeUntilNextSpawn_ = DifficultyCurve::jitteredSpawnInterval(
      sessionServed_, context.reputation(), unit(rng_));
}

void CustomerManager::spawnCustomer(SalonContext &context,
                                    CustomerEvents &events) {
  using namespace DifficultyCurve;

  const int waitSlotIndex = findFreeWaitSlot(context);
  if (waitSlotIndex < 0) {
    SAL_LOGW("Salon: no free wait slot, spawn rejected");
    events.spawnRejected = true;
    return;
  }

  const CustomerArchetype *arch =
      CustomerCatalog::pickForReputation(context.reputation(), rng_);
  if (arch == nullptr)
    return;

  ++nextCustomerId_;

  const float patience = kBasePatienceSeconds * arch->patienceMultiplier *
                         patienceMultiplier(sessionServed_);

  const math::Vec2 spawnPos = SalonLayout::cellCenter(
      SalonLayout::kDoorCell.col, SalonLayout::kDoorCell.row);
  const math::Vec2 waitPos = SalonLayout::cellCenterByIndex(waitSlotIndex);

  Customer customer(nextCustomerId_, CustomerState::Arriving, spawnPos,
                    arch->id, patience, arch->tipMultiplier);
  customer.setTargetPosition(waitPos);
  customer.setReserveCellIndex(waitSlotIndex);

  reserveCell(context, waitSlotIndex, customer.id());
  customers_.push_back(customer);
  ++events.spawned;

  SAL_LOGI("Salon: spawning customer %u", nextCustomerId_);
}

void CustomerManager::applyCustomerTransition(Customer &customer,
                                              SalonContext &context,
                                              CustomerEvents &events) {
  using L = SalonLayout;
  switch (customer.currentState()) {
  case CustomerState::Arriving:
    if (customer.hasArrived()) {
      customer.setCurrentState(CustomerState::Queueing);
    }
    break;

  case CustomerState::Queueing:
    if (customer.hasGivenUp()) {
      freeCustomerCell(context, customer);
      customer.setTargetPosition(
          L::cellCenter(L::kDoorCell.col, L::kDoorCell.row));
      customer.setCurrentState(CustomerState::Leaving);
      customer.setOutcome(CustomerOutcome::GaveUp);
    }
    break;

  case CustomerState::WalkingToSeat:
    if (customer.hasArrived()) {
      customer.setCurrentState(CustomerState::Seated);
    }
    break;

  case CustomerState::Seated:
    if (customer.hasSatLongEnough()) {
      freeCustomerCell(context, customer);
      customer.setTargetPosition(
          L::cellCenter(L::kDoorCell.col, L::kDoorCell.row));
      customer.setOutcome(CustomerOutcome::Served);
      customer.setCurrentState(CustomerState::Leaving);
      events.completedAt.push_back(customer.currentPosition());
    }
    break;

  case CustomerState::Leaving:
    break;
  }
}

void CustomerManager::removeGoneCustomers(SalonContext &context,
                                          CustomerEvents &events) {
  auto it = customers_.begin();
  while (it != customers_.end()) {
    if (it->currentState() == CustomerState::Leaving && it->hasArrived()) {
      freeCustomerCell(context, *it);
      switch (it->outcome()) {
      case CustomerOutcome::Served: {
        const int32_t payout = static_cast<int32_t>(
            DifficultyCurve::kBasePayout * it->tipMultiplier());
        context.onCustomerServed(payout);
        ++sessionServed_;
        events.servedAt.push_back(it->currentPosition());
        break;
      }
      case CustomerOutcome::GaveUp:
        ++sessionLost_;
        context.onCustomerLost();
        ++events.gaveUp;
        break;
      }
      it = customers_.erase(it);
    } else {
      ++it;
    }
  }
}

int CustomerManager::findFreeWaitSlot(const SalonContext &context) const {
  using L = SalonLayout;
  const SalonLayout &layout = context.layout();
  for (const auto &w : L::kWaitSlots) {
    if (layout.cellAt(w.col, w.row).occupantId == 0) {
      return L::cellIndex(w.col, w.row);
    }
  }
  return -1;
}

int CustomerManager::findFreeStation(const SalonContext &context) const {
  using L = SalonLayout;
  const SalonLayout &layout = context.layout();
  for (const auto &s : layout.stations()) {
    if (layout.cellAt(s.gridX, s.gridY).occupantId == 0) {
      return L::cellIndex(s.gridX, s.gridY);
    }
  }
  return -1;
}

void CustomerManager::reserveCell(SalonContext &context, int index,
                                  uint32_t customerId) {
  if (index < 0)
    return;
  context.layout()
      .cellAt(index % SalonLayout::kCols, index / SalonLayout::kCols)
      .occupantId = customerId;
}

void CustomerManager::freeCell(SalonContext &context, int index) {
  if (index < 0)
    return;
  context.layout()
      .cellAt(index % SalonLayout::kCols, index / SalonLayout::kCols)
      .occupantId = 0;
}

void CustomerManager::freeCustomerCell(SalonContext &context,
                                       Customer &customer) {
  if (customer.reservedCellIndex() < 0)
    return;
  freeCell(context, customer.reservedCellIndex());
  customer.setReserveCellIndex(-1);
}

} // namespace salshalon::features::salon
