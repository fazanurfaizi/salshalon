#pragma once

#include <memory>
#include <string>

#include "features/salon/SalonContext.hpp"
#include "features/salon/cosmetics/CosmeticCatalog.hpp"
#include "game/GameState.hpp"
#include "math/Rect.hpp"
#include "ui/Button.hpp"
#include "ui/Renderer.hpp"
#include "ui/Toast.hpp"

namespace salshalon::features::salon {

/// Build mode: place and remove stations
class SalonBuildState final : public game::GameState {
public:
  void onEnter(core::Engine &engine) override;
  void onExit() override;

  void update(float deltaSeconds) override;
  void render(graphics::SpriteBatch &batch) override;
  void onTouch(const input::TouchEvent &event) override;

private:
  enum class Category { Stations, Floors, Walls, Decor };

  // HUD
  static constexpr float kHudTopHeight = 64.0f;
  static constexpr float kHudBottomHeight = 160.0f;

  // Tabs
  static constexpr float kTabY = 12.0f;
  static constexpr float kTabWidth = 120.0f;
  static constexpr float kTabHeight = 40.0f;
  static constexpr float kTabGap = 8.0f;
  static constexpr float kTabStartX = 280.0f;

  // Item strip
  static constexpr float kItemWidth = 200.0f;
  static constexpr float kItemHeight = 96.0f;
  static constexpr float kItemGap = 16.0f;
  static constexpr float kItemStripY = 608.0f;

  // Buttons
  static constexpr float kSideButtonWidth = 160.0f;
  static constexpr float kSideButtonHeight = 72.0f;
  static constexpr float kSideButtonMargin = 24.0f;
  static constexpr float kSideButtonY = 624.0f;

  SalonContext *context_ = nullptr;
  ui::Renderer renderer_{};
  ui::Toast toast_{};

  Category activeCategory_ = Category::Stations;
  std::string selectedArchetypeId_{};

  std::unique_ptr<ui::Button> openSalonButton_{};
  std::unique_ptr<ui::Button> menuButton_{};

  // Tabs
  math::Rect tabRect(int index) const;
  int tabIndexAt(const math::Vec2 &p) const;
  void setCategory(Category c);

  // Layout
  math::Rect itemRect(int index) const;
  int itemIndexAt(const math::Vec2 &p) const;

  // Actions
  void onCellTapped(int col, int row);
  void onOpenSalonPressed();
  void onMenuPressed();

  void renderHud();
  void renderTabs();
  void renderItemStrip();
  void renderStationItems();
  void renderCosmeticItems(CosmeticKind kind);
};

} // namespace salshalon::features::salon
