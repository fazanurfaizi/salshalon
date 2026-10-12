#include "features/salon/SalonBuildState.hpp"

#include "core/Engine.hpp"
#include "core/GameConfig.hpp"
#include "core/Log.hpp"
#include "features/Feature.hpp"
#include "features/main_menu/MainMenuState.hpp"
#include "features/salon/BuildSystem.hpp"
#include "features/salon/SalonPlayState.hpp"
#include "features/salon/StationCatalog.hpp"

namespace salshalon::features::salon {

using core::kDesignHeight;
using core::kDesignWidth;

void SalonBuildState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->context_ = &engine.salon();

  const math::Rect openRect{
      kDesignWidth - kOpenSalonWidth,
      kSideButtonY,
      kOpenSalonWidth,
      kOpenSalonHeight,
  };
  this->openSalonButton_ = std::make_unique<ui::Button>(
      "OPEN SALON", openRect, [this] { this->onOpenSalonPressed(); });
  this->openSalonButton_->setTextScale(3.0f);

  const math::Rect menuRect{
      0.0f,
      kSideButtonY,
      kOpenSalonWidth,
      kOpenSalonHeight,
  };
  this->menuButton_ = std::make_unique<ui::Button>(
      "MENU", menuRect, [this] { this->onMenuPressed(); });
  this->menuButton_->setTextScale(3.0f);

  this->toast_.setKind(ui::ToastKind::Info).setFadeTime(1.5f);

  SAL_LOGI("Build: enter (money $%d, stations %zu)", this->context_->money(),
           this->context_->layout().stations().size());
}

void SalonBuildState::onExit() {
  this->context_->layout().resetOccupantId();
  this->openSalonButton_.reset();
  this->menuButton_.reset();
  this->context_ = nullptr;
  SAL_LOGI("Build: exit");
}

void SalonBuildState::update(float deltaSeconds) {
  this->openSalonButton_->update(deltaSeconds);
  this->menuButton_->update(deltaSeconds);
  this->toast_.update(deltaSeconds);
}

void SalonBuildState::render(graphics::SpriteBatch &batch) {
  this->renderer_.setBatch(&batch);

  this->renderer_.drawRect({0.0f, 0.0f, kDesignWidth, kDesignHeight},
                           graphics::Color{0.10f, 0.11f, 0.14f, 1.0f});

  this->renderGrid();
  this->renderStations();
  this->renderHud();
  this->renderItemStrip();

  this->openSalonButton_->render(this->renderer_);
  this->menuButton_->render(this->renderer_);

  this->toast_.render(
      this->renderer_,
      {kDesignWidth * 0.5f, kDesignHeight - kHudBottomHeight - 40.0f},
      ui::ToastAnchor::BottomCenter);
}

void SalonBuildState::renderGrid() {
  using L = SalonLayout;

  // Floor background under the tiles
  const math::Rect floorRect{
      L::kOriginX,
      L::kOriginY,
      L::kCols * L::kTileSize,
      L::kRows * L::kTileSize,
  };
  this->renderer_.drawRect(floorRect,
                           graphics::Color{0.16f, 0.13f, 0.18f, 1.0f});

  const SalonLayout &layout = this->context_->layout();

  // Checkerboard tiles
  for (int row = 0; row < L::kRows; ++row) {
    for (int col = 0; col < L::kCols; ++col) {
      const math::Rect cellRect{
          L::kOriginX + col * L::kTileSize,
          L::kOriginY + row * L::kTileSize,
          L::kTileSize,
          L::kTileSize,
      };

      const bool alternate = ((col + row) & 1) != 0;
      const Cell &cell = layout.cellAt(col, row);
      this->renderer_.drawRect(cellRect.inset(1.0f),
                               cellColor(cell.type, alternate));
    }
  }
}

void SalonBuildState::renderStations() {
  using L = SalonLayout;
  const SalonLayout &layout = this->context_->layout();

  for (const auto &inst : layout.stations()) {
    const math::Rect cellRect{
        L::kOriginX + inst.gridX * L::kTileSize,
        L::kOriginY + inst.gridY * L::kTileSize,
        L::kTileSize,
        L::kTileSize,
    };

    const StationArchetype *arch = StationCatalog::find(inst.archetypeId);
    const graphics::Color color =
        arch ? arch->placeholderColor : graphics::Color{0.7f, 0.7f, 0.7f, 1.0f};

    this->renderer_.drawRect(cellRect.inset(8.0f), color);
    this->renderer_.drawFrame(cellRect.inset(8.0f),
                              graphics::Color{1.0f, 1.0f, 1.0f, 0.30}, 2.0f);
  }
}

void SalonBuildState::renderHud() {
  this->renderer_.drawRect({0.0f, 0.0f, kDesignWidth, kHudTopHeight},
                           graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});

  this->renderer_.drawText(
      "BUILD - DAY " + std::to_string(this->context_->day()),
      {24.0f, kHudTopHeight * 0.5f - 12.0f}, 2.5f, graphics::kWhite);

  const std::string moneyStr = "$" + std::to_string(this->context_->money());
  const float moneyWidth = this->renderer_.measureText(moneyStr, 2.5f);
  this->renderer_.drawText(
      moneyStr,
      {kDesignWidth - 24.0f - moneyWidth, kHudTopHeight * 0.5f - 12.0f}, 2.5f,
      graphics::Color{0.85f, 1.0f, 0.75f, 1.0f});

  // Bottom strip
  this->renderer_.drawRect(
      {0.0f, kDesignHeight - kHudBottomHeight, kDesignWidth, kHudBottomHeight},
      graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});
}

void SalonBuildState::renderItemStrip() {
  const auto &catalog = StationCatalog::all();

  for (int i = 0; i < static_cast<int>(catalog.size()); ++i) {
    const math::Rect rect = this->itemRect(i);
    const StationArchetype &arch = catalog[i];

    const bool selected = (arch.id == this->selectedArchetypeId_);
    const bool affordable = this->context_->money() >= arch.cost;

    const graphics::Color fill =
        selected ? graphics::Color{0.30f, 0.55f, 0.85f, 1.0f}
                 : graphics::Color{0.15f, 0.17f, 0.23f, 1.0f};

    this->renderer_.drawRect(rect, fill);
    // clang-format off
    this->renderer_.drawFrame(
      rect,
      affordable
        ? graphics::Color{1.0f, 1.0f, 1.0f, 0.35f}
        : graphics::Color{0.4f, 0.4f, 0.4f, 0.35f},
      2.0f
    );
    // clang-format on

    // Swatch
    const math::Rect swatch{rect.x + 12.0f, rect.y + 12.0f, 40.0f, 40.0f};
    this->renderer_.drawRect(swatch, arch.placeholderColor);
    this->renderer_.drawFrame(swatch, graphics::Color{1, 1, 1, 0.25f}, 1.0f);

    // Label
    // clang-format off
    this->renderer_.drawText(
      arch.displayName,
      {rect.x + 64.0f, rect.y + 12.0f},
      2.6f,
      graphics::kWhite
    );

    // Price
    const std::string priceStr = "$" + std::to_string(arch.cost);
    this->renderer_.drawText(
      priceStr, 
      {rect.x + 64.0f, rect.y + 48.0f},
      2.2f,
      affordable 
        ? graphics::Color{0.85f, 1.0f, 0.75f, 1.0f}
        : graphics::Color{1.0f, 0.55f, 0.55f, 1.0f}
    );
    // clang-format on
  }
}

// Item strip geometry
math::Rect SalonBuildState::itemRect(int index) const {
  // Available horizontal space: between the two side buttons.
  const float leftEdge = kSideButtonMargin + kSideButtonWidth + 20.0f;
  const float rightEdge =
      core::kDesignWidth - kSideButtonMargin - kSideButtonWidth - 20.0f;
  const float available = rightEdge - leftEdge;

  const int count = static_cast<int>(StationCatalog::all().size());
  const float stripWidth = count * kItemWidth + (count - 1) * kItemGap;
  const float startX = leftEdge + (available - stripWidth) * 0.5f;

  return {startX + index * (kItemWidth + kItemGap), kItemStripY, kItemWidth,
          kItemHeight};
}

int SalonBuildState::itemIndexAt(const math::Vec2 &p) const {
  const int count = static_cast<int>(StationCatalog::all().size());
  for (int i = 0; i < count; ++i) {
    if (itemRect(i).contains(p))
      return i;
  }
  return -1;
}

void SalonBuildState::onTouch(const input::TouchEvent &event) {
  if (this->openSalonButton_->onTouch(event))
    return;
  if (this->menuButton_->onTouch(event))
    return;
  if (event.phase != input::TouchPhase::Down)
    return;

  // Item strip - select an archetype
  const int itemIdx = this->itemIndexAt(event.position);
  if (itemIdx >= 0) {
    const auto &arch = StationCatalog::all()[itemIdx];
    if (this->selectedArchetypeId_ == arch.id) {
      this->selectedArchetypeId_.clear(); // tap again to deselect
    } else {
      this->selectedArchetypeId_ = arch.id;
    }
    return;
  }

  // Grid cell
  int col = 0, row = 0;
  if (!SalonLayout::cellFromWorld(event.position, col, row))
    return;

  this->onCellTapped(col, row);
}

void SalonBuildState::onCellTapped(int col, int row) {
  SalonLayout &layout = this->context_->layout();

  // Nothing selected - tap a station to sell it.
  if (this->selectedArchetypeId_.empty()) {
    const auto result = BuildSystem::remove(*this->context_, col, row);
    if (result == BuildSystem::Result::NothingToRemove)
      return;
    this->toast_
        .setKind(result == BuildSystem::Result::Success
                     ? ui::ToastKind::Success
                     : ui::ToastKind::Warning)
        .setTitle(result == BuildSystem::Result::Success ? "Sold"
                                                         : "Can't sell")
        .show(BuildSystem::resultMessage(result));
    return;
  }

  // Something selected - try to place it
  const auto result =
      BuildSystem::place(*this->context_, this->selectedArchetypeId_, col, row);

  if (result == BuildSystem::Result::Success) {
    this->selectedArchetypeId_.clear();
    return;
  }
  this->toast_.setKind(ui::ToastKind::Warning)
      .setTitle("Can't place")
      .show(BuildSystem::resultMessage(result));
}

void SalonBuildState::onOpenSalonPressed() {
  SAL_LOGI("Build: OPEN SALON");
  goTo<SalonPlayState>(*this->engine_);
}

void SalonBuildState::onMenuPressed() {
  SAL_LOGI("Build: MENU");
  goTo<main_menu::MainMenuState>(*this->engine_);
}

graphics::Color SalonBuildState::cellColor(CellType type,
                                           bool alternate) const {
  switch (type) {
  case CellType::Door:
    return {0.45f, 0.25f, 0.15f, 1.0f};
  case CellType::WaitSlot:
    return {0.20f, 0.40f, 0.55f, 1.0f};
  case CellType::Station:
    return {0.30f, 0.24f, 0.32f, 1.0f};
  case CellType::Empty:
  default:
    return alternate ? graphics::Color{0.21f, 0.17f, 0.24f, 1.0f}
                     : graphics::Color{0.18f, 0.15f, 0.21f, 1.0f};
  }
}

} // namespace salshalon::features::salon
