#include <abysstoolkit/core/Backend.hpp>
#include <abysstoolkit/window/Window.hpp>
#include <abysstoolkit/element/Rectangle.hpp>
#include <abysstoolkit/element/RowLayout.hpp>
#include <abysstoolkit/element/ColumnLayout.hpp>
#include <abysstoolkit/element/Text.hpp>
#include <abysstoolkit/element/Image.hpp>
#include <abysstoolkit/element/Button.hpp>
#include <abysstoolkit/element/Null.hpp>

#include <abyssutils/memory/SharedPtr.hpp>
#include <abyssutils/memory/UniquePtr.hpp>
#include <abyssutils/string/ConstVarList.hpp>
#include <abyssutils/string/String.hpp>
#include <abyssutils/os/Process.hpp>

#include <print>

using namespace Abyssutils::Memory;
using namespace Abyssutils::Math;
using namespace Abyssutils::String;
using namespace Abyssutils::OS;
using namespace Abysstoolkit;

#define SP CSharedPointer
#define WP CWeakPointer
#define UP CUniquePointer

static SP<IBackend>       backend;
static SP<CButtonElement> donateButton;

constexpr const char*     SUPPORT_MESSAGE = R"#(Abyssland is built and maintained by volunteers, with one person working on it full-time.

If Abyssland is useful to you, supporting the project helps keep that work sustainable and lets us keep improving it.

You can make a one-time donation or support us monthly.
There's also Abyssperks for 5€ + tax / month, which includes a few small "thank you" goodies from us.

)#";

//
int main(int argc, char** argv, char** envp) {
    setenv("HT_QUIET", "1", true);
    backend = IBackend::create();

    const auto FONT_SIZE   = CFontSize{CFontSize::HT_FONT_TEXT}.ptSize();
    const auto WINDOW_SIZE = Vector2D{FONT_SIZE * 70.F, FONT_SIZE * 26.F};

    auto       window = CWindowBuilder::begin()
                            ->preferredSize(WINDOW_SIZE)
                            ->minSize(WINDOW_SIZE)
                            ->maxSize(WINDOW_SIZE)
                            ->appTitle("Support Abyssland")
                            ->appClass("abyssland-donate-screen")
                            ->commence();

    window->m_rootElement->addChild(CRectangleBuilder::begin()->color([] { return backend->getPalette()->m_colors.background; })->commence());

    auto layout = CColumnLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_PERCENT, {1.F, 1.F}})->commence();
    layout->setMargin(3);

    auto layoutInner = CColumnLayoutBuilder::begin()->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {0.85F, 1.F}})->commence();

    window->m_rootElement->addChild(layout);

    layout->addChild(layoutInner);
    layoutInner->setGrow(true);
    layoutInner->setPositionMode(Abysstoolkit::IElement::HT_POSITION_ABSOLUTE);
    layoutInner->setPositionFlag(Abysstoolkit::IElement::HT_POSITION_FLAG_HCENTER, true);

    auto title = CTextBuilder::begin()->text("Support Abyssland")->fontSize({CFontSize::HT_FONT_H2})->color([] { return backend->getPalette()->m_colors.text; })->commence();

    auto hr = CRectangleBuilder::begin() //
                  ->color([] { return CAbyssColor{backend->getPalette()->m_colors.text.darken(0.65)}; })
                  ->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_ABSOLUTE, {0.5F, 9.F}})
                  ->commence();

    hr->setMargin(4);

    auto content = CTextBuilder::begin()
                       ->text(SUPPORT_MESSAGE)
                       ->fontSize(CFontSize{CFontSize::HT_FONT_TEXT})
                       ->color([] { return backend->getPalette()->m_colors.text; })
                       ->align(Abysstoolkit::HT_FONT_ALIGN_CENTER)
                       ->size(CDynamicSize{CDynamicSize::HT_SIZE_PERCENT, Abysstoolkit::CDynamicSize::HT_SIZE_AUTO, {1, 1}})
                       ->commence();

    content->setPositionFlag(Abysstoolkit::IElement::HT_POSITION_FLAG_HCENTER, true);
    content->setGrow(true);

    donateButton = CButtonBuilder::begin()
                       ->label("💝 Support")
                       ->onMainClick([w = WP<IWindow>{window}](auto) {
                           donateButton->rebuild()->label("💝 Thank you!")->commence();

                           CProcess proc("xdg-open", {"https://github.com/AbyssWM"});
                           proc.runAsync();
                       })
                       ->size({CDynamicSize::HT_SIZE_AUTO, CDynamicSize::HT_SIZE_AUTO, {1, 1}})
                       ->commence();

    auto noThanksButton = CButtonBuilder::begin()
                              ->label("No thanks")
                              ->onMainClick([w = WP<IWindow>{window}](auto) {
                                  if (w)
                                      w->close();
                                  backend->destroy();
                              })
                              ->size({CDynamicSize::HT_SIZE_AUTO, CDynamicSize::HT_SIZE_AUTO, {1, 1}})
                              ->commence();

    auto null2 = CNullBuilder::begin()->commence();

    auto layout2 = CRowLayoutBuilder::begin()->gap(3)->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {1, 1}})->commence();

    null2->setGrow(true);

    layoutInner->addChild(title);
    layoutInner->addChild(hr);
    layoutInner->addChild(content);

    layout2->addChild(null2);
    layout2->addChild(donateButton);
    layout2->addChild(noThanksButton);

    layout->addChild(layout2);

    window->m_events.closeRequest.listenStatic([w = WP<IWindow>{window}] {
        w->close();
        backend->destroy();
    });

    window->open();

    backend->enterLoop();

    return 0;
}