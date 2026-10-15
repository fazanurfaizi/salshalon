#include "features/salon/SalonBuildState.hpp"

#include "audio/Sfx.hpp"
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

namespace {
constexpr int kCategoryCount = 4;
}

void SalonBuildState::onEnter(core::Engine &engine) {
  game::GameState::onEnter(engine);
  this->context_ = &engine.salon();

  const math::Rect openRect{
      kDesignWidth - kSideButtonMargin - kSideButtonWidth,
      kSideButtonY,
      kSideButtonWidth,
      kSideButtonHeight,
  };
  openSalonButton_ = std::make_unique<ui::Button>(
      "OPEN SALON", openRect, [this] { onOpenSalonPressed(); });
  openSalonButton_->setTextScale(3.0f);

  const math::Rect menuRect{
      kSideButtonMargin,
      kSideButtonY,
      kSideButtonWidth,
      kSideButtonHeight,
  };
  menuButton_ = std::make_unique<ui::Button>("MENU", menuRect,
                                             [this] { onMenuPressed(); });
  menuButton_->setTextScale(3.0f);

  toast_.setKind(ui::ToastKind::Info).setFadeTime(1.5f);

  SAL_LOGI("Build: enter (money $%d, stations %zu, decor %zu)",
           context_->money(), context_->layout().stations().size(),
           context_->layout().decors().size());
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

  // Wall background
  const auto *wallArch = CosmeticCatalog::find(this->context_->currentWallId());
  const graphics::Color wallColor =
      wallArch ? wallArch->primary : graphics::Color{0.30f, 0.26f, 0.22f, 1.0f};
  this->renderer_.drawRect({0.0f, 0.0f, kDesignWidth, kDesignHeight},
                           wallColor);

  this->renderGrid();
  this->renderStations();
  this->renderDecor();
  this->renderHud();
  this->renderTabs();
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

  const auto *floorArch =
      CosmeticCatalog::find(this->context_->currentFloorId());
  const graphics::Color tileA =
      floorArch ? floorArch->primary
                : graphics::Color{0.20f, 0.20f, 0.14f, 1.0f};
  const graphics::Color tileB =
      floorArch ? floorArch->secondary
                : graphics::Color{0.32f, 0.23f, 0.16f, 1.0f};

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

      graphics::Color fill;
      if (cell.type == CellType::Empty) {
        fill = alternate ? tileB : tileA;
      } else {
        fill = cellColor(cell.type, alternate);
      }
      this->renderer_.drawRect(cellRect.inset(1.0f), fill);
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
  renderer_.drawRect({0.0f, 0.0f, kDesignWidth, kHudTopHeight},
                     graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});

  renderer_.drawText("BUILD - DAY " + std::to_string(context_->day()),
                     {24.0f, kHudTopHeight * 0.5f - 12.0f}, 2.5f,
                     graphics::kWhite);

  const std::string moneyStr = "$" + std::to_string(context_->money());
  const float moneyWidth = renderer_.measureText(moneyStr, 2.5f);
  renderer_.drawText(
      moneyStr,
      {kDesignWidth - 24.0f - moneyWidth, kHudTopHeight * 0.5f - 12.0f}, 2.5f,
      graphics::Color{0.85f, 1.0f, 0.75f, 1.0f});

  renderer_.drawRect(
      {0.0f, kDesignHeight - kHudBottomHeight, kDesignWidth, kHudBottomHeight},
      graphics::Color{0.04f, 0.04f, 0.07f, 1.0f});
}

void SalonBuildState::renderTabs() {
  static const char *kLabels[kCategoryCount] = {"STATIONS", "FLOORS", "WALLS",
                                                "DECOR"};

  for (int i = 0; i < kCategoryCount; ++i) {
    const math::Rect r = this->tabRect(i);
    const bool selected = (static_cast<int>(this->activeCategory_) == i);

    this->renderer_.drawRect(
        r, selected ? graphics::Color{0.30f, 0.55f, 0.85f, 1.0f}
                    : graphics::Color{0.15f, 0.17f, 0.23f, 1.0f});
    this->renderer_.drawFrame(
        r, graphics::Color{1.0f, 1.0f, 1.0f, selected ? 0.5f : 0.20f}, 2.0f);

    const float textW = this->renderer_.measureText(kLabels[i], 2.0f);
    this->renderer_.drawText(kLabels[i],
                             {r.center().x - textW * 0.5f, r.y + 10.0f}, 2.0f,
                             graphics::kWhite);
  }
}

void SalonBuildState::renderItemStrip() {
  switch (this->activeCategory_) {
  case Category::Stations:
    this->renderStationItems();
    break;
  case Category::Floors:
    this->renderCosmeticItems(CosmeticKind::Floor);
    break;
  case Category::Walls:
    this->renderCosmeticItems(CosmeticKind::Wall);
    break;
  case Category::Decor:
    this->renderCosmeticItems(CosmeticKind::Decor);
    break;
  }
}

void SalonBuildState::renderStationItems() {
  const auto &catalog = StationCatalog::all();

  for (int i = 0; i < static_cast<int>(catalog.size()); ++i) {
    const math::Rect rect = this->itemRect(i);
    const StationArchetype &arch = catalog[i];

    const bool selected = (arch.id == this->selectedArchetypeId_);
    const bool affordable = this->context_->money() >= arch.cost;

    this->renderer_.drawRect(
        rect, selected ? graphics::Color{0.30f, 0.55f, 0.85f, 1.0f}
                       : graphics::Color{0.15f, 0.17f, 0.23f, 1.0f});
    this->renderer_.drawFrame(rect,
                              affordable
                                  ? graphics::Color{1.0f, 1.0f, 1.0f, 0.35f}
                                  : graphics::Color{0.4f, 0.4f, 0.4f, 0.35f},
                              2.0f);

    const math::Rect swatch{rect.x + 12.0f, rect.y + 12.0f, 40.0f, 40.0f};
    this->renderer_.drawRect(swatch, arch.placeholderColor);
    this->renderer_.drawFrame(swatch, graphics::Color{1, 1, 1, 0.25f}, 1.0f);

    this->renderer_.drawText(arch.displayName, {rect.x + 64.0f, rect.y + 12.0f},
                             2.6f, graphics::kWhite);

    const std::string priceStr = "$" + std::to_string(arch.cost);
    this->renderer_.drawText(priceStr, {rect.x + 64.0f, rect.y + 48.0f}, 2.2f,
                             affordable
                                 ? graphics::Color{0.85f, 1.0f, 0.75f, 1.0f}
                                 : graphics::Color{1.0f, 0.55f, 0.55f, 1.0f});
  }
}

void SalonBuildState::renderCosmeticItems(CosmeticKind kind) {
  const auto items = CosmeticCatalog::byKind(kind);

  for (int i = 0; i < static_cast<int>(items.size()); ++i) {
    const math::Rect rect = this->itemRect(i);
    const CosmeticArchetype *arch = items[i];
    const bool unlocked = this->context_->isCosmeticUnlocked(arch->id);

    // Determine selection: Floors/Walls show current, Decor shows selected.
    bool selected = false;
    if (kind == CosmeticKind::Floor) {
      selected = (this->context_->currentFloorId() == arch->id);
    } else if (kind == CosmeticKind::Wall) {
      selected = (this->context_->currentWallId() == arch->id);
    } else if (kind == CosmeticKind::Decor) {
      selected = (this->selectedArchetypeId_ == arch->id);
    }

    const graphics::Color fill =
        !unlocked  ? graphics::Color{0.10f, 0.10f, 0.12f, 1.0f}
        : selected ? graphics::Color{0.30f, 0.55f, 0.85f, 1.0f}
                   : graphics::Color{0.15f, 0.17f, 0.23f, 1.0f};

    this->renderer_.drawRect(rect, fill);
    this->renderer_.drawFrame(rect,
                              unlocked
                                  ? graphics::Color{1.0f, 1.0f, 1.0f, 0.35f}
                                  : graphics::Color{0.4f, 0.4f, 0.4f, 0.35f},
                              2.0f);

    const math::Rect swatch{rect.x + 12.0f, rect.y + 12.0f, 40.0f, 40.0f};
    const graphics::Color swatchColor =
        unlocked ? arch->primary : graphics::Color{0.25f, 0.25f, 0.20f, 1.0f};
    this->renderer_.drawRect(swatch, swatchColor);
    this->renderer_.drawFrame(swatch, graphics::Color{1, 1, 1, 0.25f}, 1.0f);

    const graphics::Color textColor =
        unlocked ? graphics::kWhite : graphics::Color{0.5f, 0.5f, 0.55f, 1.0f};

    this->renderer_.drawText(arch->displayName,
                             {rect.x + 64.0f, rect.y + 12.0f}, 2.6f, textColor);

    if (!unlocked) {
      this->renderer_.drawText("Locked", {rect.x + 64.0f, rect.y + 48.0f}, 2.2f,
                               graphics::Color{0.55f, 0.55f, 0.60f, 1.0f});
    }
  }
}

void SalonBuildState::renderDecor() {
  using L = SalonLayout;
  for (const auto &d : this->context_->layout().decors()) {
    const math::Rect cellRect{
        L::kOriginX + d.gridX * L::kTileSize,
        L::kOriginY + d.gridY * L::kTileSize,
        L::kTileSize,
        L::kTileSize,
    };
    const auto *arch = CosmeticCatalog::find(d.archetypeId);
    const graphics::Color c =
        arch ? arch->primary : graphics::Color{0.7f, 0.7f, 0.7f, 1.0f};
    this->renderer_.drawRect(cellRect.inset(L::kTileSize * 0.20f), c);
    this->renderer_.drawFrame(cellRect.inset(L::kTileSize * 0.20f),
                              graphics::Color{1.0f, 1.0f, 1.0f, 0.25f}, 2.0f);
  }
}

// Item strip geometry
math::Rect SalonBuildState::itemRect(int index) const {
  // Available horizontal space: between the two side buttons.
  const float leftEdge = kSideButtonMargin + kSideButtonWidth + 20.0f;
  const float rightEdge =
      core::kDesignWidth - kSideButtonMargin - kSideButtonWidth - 20.0f;
  const float available = rightEdge - leftEdge;

  int count = 0;
  switch (this->activeCategory_) {
  case Category::Stations:
    count = static_cast<int>(StationCatalog::all().size());
    break;
  case Category::Floors:
    count =
        static_cast<int>(CosmeticCatalog::byKind(CosmeticKind::Floor).size());
    break;
  case Category::Walls:
    count =
        static_cast<int>(CosmeticCatalog::byKind(CosmeticKind::Wall).size());
    break;
  case Category::Decor:
    count =
        static_cast<int>(CosmeticCatalog::byKind(CosmeticKind::Decor).size());
    break;
  }

  const float stripWidth =
      count * kItemWidth + std::max(0, count - 1) * kItemGap;
  const float startX = leftEdge + (available - stripWidth) * 0.5f;

  return {startX + index * (kItemWidth + kItemGap), kItemStripY, kItemWidth,
          kItemHeight};
}

math::Rect SalonBuildState::tabRect(int index) const {
  return {kTabStartX + index * (kTabWidth + kTabGap), kTabY, kTabWidth,
          kTabHeight};
}

int SalonBuildState::tabIndexAt(const math::Vec2 &p) const {
  for (int i = 0; i < kCategoryCount; ++i) {
    if (this->tabRect(i).contains(p))
      return i;
  }
  return -1;
}

int SalonBuildState::itemIndexAt(const math::Vec2 &p) const {
  int count = 0;
  switch (this->activeCategory_) {
  case Category::Stations:
    count = static_cast<int>(StationCatalog::all().size());
    break;
  case Category::Floors:
    count =
        static_cast<int>(CosmeticCatalog::byKind(CosmeticKind::Floor).size());
    break;
  case Category::Walls:
    count =
        static_cast<int>(CosmeticCatalog::byKind(CosmeticKind::Wall).size());
    break;
  case Category::Decor:
    count =
        static_cast<int>(CosmeticCatalog::byKind(CosmeticKind::Decor).size());
    break;
  }

  for (int i = 0; i < count; ++i) {
    if (this->itemRect(i).contains(p))
      return i;
  }
  return -1;
}

void SalonBuildState::setCategory(Category c) {
  this->activeCategory_ = c;
  this->selectedArchetypeId_.clear();
}

void SalonBuildState::onTouch(const input::TouchEvent &event) {
  if (this->openSalonButton_->onTouch(event))
    return;
  if (this->menuButton_->onTouch(event))
    return;
  if (event.phase != input::TouchPhase::Down)
    return;

  const int tabIdx = this->tabIndexAt(event.position);
  if (tabIdx >= 0) {
    this->setCategory(static_cast<Category>(tabIdx));
    return;
  }

  // Item strip - select an archetype
  const int itemIdx = this->itemIndexAt(event.position);
  if (itemIdx >= 0) {
    switch (this->activeCategory_) {
    case Category::Stations: {
      const auto &arch = StationCatalog::all()[itemIdx];
      this->selectedArchetypeId_ =
          (this->selectedArchetypeId_ == arch.id) ? "" : arch.id;
      break;
    }
    case Category::Floors: {
      const auto items = CosmeticCatalog::byKind(CosmeticKind::Floor);
      const auto *arch = items[itemIdx];
      if (!this->context_->isCosmeticUnlocked(arch->id)) {
        this->toast_.setKind(ui::ToastKind::Warning)
            .setTitle("Locked")
            .show("Keep playing to unlock");
        return;
      }
      this->context_->setCurrentFloor(arch->id);
      this->engine_->saveProfile();
      this->toast_.setKind(ui::ToastKind::Success)
          .setTitle("Floor changed")
          .show(arch->displayName);
      break;
    }
    case Category::Walls: {
      const auto items = CosmeticCatalog::byKind(CosmeticKind::Wall);
      const auto *arch = items[itemIdx];
      if (!this->context_->isCosmeticUnlocked(arch->id)) {
        this->toast_.setKind(ui::ToastKind::Warning)
            .setTitle("Locked")
            .show("Keep playing to unlock");
        return;
      }
      this->context_->setCurrentWall(arch->id);
      this->engine_->saveProfile();
      this->toast_.setKind(ui::ToastKind::Success)
          .setTitle("Wall changed")
          .show(arch->displayName);
      break;
    }
    case Category::Decor: {
      const auto items = CosmeticCatalog::byKind(CosmeticKind::Decor);
      const auto *arch = items[itemIdx];
      if (!this->context_->isCosmeticUnlocked(arch->id)) {
        this->toast_.setKind(ui::ToastKind::Warning)
            .setTitle("Locked")
            .show("Keep playing to unlock");
        return;
      }
      this->selectedArchetypeId_ =
          (this->selectedArchetypeId_ == arch->id) ? "" : arch->id;
      break;
    }
    }
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
  if (this->activeCategory_ == Category::Decor) {
    if (this->selectedArchetypeId_.empty()) {
      const auto removed = layout.removeDecorAt(col, row);
      if (removed) {
        this->engine_->saveProfile();
        this->toast_.setKind(ui::ToastKind::Success)
            .setTitle("Removed")
            .show("");
      }
      return;
    }
    DecorInstance d;
    d.archetypeId = this->selectedArchetypeId_;
    d.gridX = col;
    d.gridY = row;
    if (layout.placeDecor(d)) {
      this->selectedArchetypeId_.clear();
      this->engine_->saveProfile();
    } else {
      this->toast_.setKind(ui::ToastKind::Warning)
          .setTitle("Can't place here")
          .show("");
    }
    return;
  }

  // Floors/Walls not use grid taps.
  if (this->activeCategory_ == Category::Floors ||
      this->activeCategory_ == Category::Walls) {
    return;
  }

  // Stations
  if (this->selectedArchetypeId_.empty()) {
    const auto result = BuildSystem::remove(*this->context_, col, row);
    if (result == BuildSystem::Result::NothingToRemove)
      return;
    if (result == BuildSystem::Result::Success)
      this->engine_->saveProfile();
    this->toast_
        .setKind(result == BuildSystem::Result::Success
                     ? ui::ToastKind::Success
                     : ui::ToastKind::Warning)
        .setTitle(result == BuildSystem::Result::Success ? "Sold"
                                                         : "Can't sell")
        .show(BuildSystem::resultMessage(result));
    return;
  }

  const auto result =
      BuildSystem::place(*this->context_, this->selectedArchetypeId_, col, row);
  if (result == BuildSystem::Result::Success) {
    this->selectedArchetypeId_.clear();
    this->engine_->saveProfile();
    return;
  }
  this->toast_.setKind(ui::ToastKind::Warning)
      .setTitle("Can't place")
      .show(BuildSystem::resultMessage(result));
}

void SalonBuildState::onOpenSalonPressed() {
  SAL_LOGI("Build: OPEN SALON");
  if (this->engine_ != nullptr)
    this->engine_->audio().play(audio::Sfx::Tap);
  goTo<SalonPlayState>(*this->engine_);
}

void SalonBuildState::onMenuPressed() {
  SAL_LOGI("Build: MENU");
  if (this->engine_ != nullptr)
    this->engine_->audio().play(audio::Sfx::Tap);
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
