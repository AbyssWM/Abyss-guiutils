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
static SP<CButtonElement> donateButton, openNewsButton;

constexpr const char*     TEXT_CONTENT = R"#(Abyssland has been updated! 😄

Check out the release notes on GitHub and the news page on abyss.land to see what's new.

Some releases include breaking changes, so if you run into any config errors, the latest release notes are a good place to start.

If you use plugins, make sure to rebuild them.

<i>You can turn off this screen in your Abyssland config.</i>)#";

//
int main(int argc, char** argv, char** envp) {
    std::string              versionStr = "";
    std::vector<std::string> buttonsStrs;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--new-version") {
            if (i + 1 >= argc) {
                std::print(stderr, "--new-version requires a parameter\n");
                return 1;
            }

            versionStr = argv[i + 1];

            i++;
            continue;
        }

        if (arg == "--help" || arg == "-h") {
            std::println(R"#(abyssland-update-screen, part of abyss-guiutils v{}
Usage:
--new-version                           - Set the displayed new version string
)#",
                         GUIUTILS_VERSION);
            return 0;
        }

        std::print(stderr, "invalid arg {}\n", argv[i]);
        return 1;
    }

    setenv("HT_QUIET", "1", true);
    backend = IBackend::create();

    //
    const auto FONT_SIZE   = CFontSize{CFontSize::HT_FONT_TEXT}.ptSize();
    const auto WINDOW_SIZE = Vector2D{FONT_SIZE * 90.F, FONT_SIZE * 30.F};

    auto       window = CWindowBuilder::begin()
                            ->preferredSize(WINDOW_SIZE)
                            ->minSize(WINDOW_SIZE)
                            ->maxSize(WINDOW_SIZE)
                            ->appTitle("Abyssland Updated")
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

    auto title = CTextBuilder::begin()
                     ->text(std::move(std::format("Abyssland updated to {}!", versionStr)))
                     ->fontSize({CFontSize::HT_FONT_H2})
                     ->color([] { return backend->getPalette()->m_colors.text; })
                     ->commence();

    auto hr = CRectangleBuilder::begin() //
                  ->color([] { return CAbyssColor{backend->getPalette()->m_colors.text.darken(0.65)}; })
                  ->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_ABSOLUTE, {0.5F, 9.F}})
                  ->commence();

    hr->setMargin(4);

    auto content = CTextBuilder::begin()
                       ->text(TEXT_CONTENT)
                       ->fontSize(CFontSize{CFontSize::HT_FONT_TEXT})
                       ->align(Abysstoolkit::HT_FONT_ALIGN_CENTER)
                       ->color([] { return backend->getPalette()->m_colors.text; })
                       ->commence();

    content->setPositionFlag(Abysstoolkit::IElement::HT_POSITION_FLAG_HCENTER, true);

    auto null2 = CNullBuilder::begin()->commence();

    auto layout2 = CRowLayoutBuilder::begin()->gap(3)->size({CDynamicSize::HT_SIZE_PERCENT, CDynamicSize::HT_SIZE_AUTO, {1, 1}})->commence();

    donateButton = CButtonBuilder::begin()
                       ->label("💝 Support")
                       ->onMainClick([w = WP<IWindow>{window}](auto) {
                           donateButton->rebuild()->label("💝 Thank you!")->commence();

                           CProcess proc("xdg-open", {"https://github.com/AbyssWM"});
                           proc.runAsync();
                       })
                       ->size({CDynamicSize::HT_SIZE_AUTO, CDynamicSize::HT_SIZE_AUTO, {1, 1}})
                       ->commence();

    openNewsButton = CButtonBuilder::begin()
                         ->label("🔗 Open news")
                         ->onMainClick([w = WP<IWindow>{window}](auto) {
                             openNewsButton->rebuild()->label("🔗 Right away!")->commence();

                             CProcess proc("xdg-open", {"https://github.com/AbyssWM"});
                             proc.runAsync();
                         })
                         ->size({CDynamicSize::HT_SIZE_AUTO, CDynamicSize::HT_SIZE_AUTO, {1, 1}})
                         ->commence();

    auto closeButton = CButtonBuilder::begin()
                           ->label("Thanks")
                           ->onMainClick([w = WP<IWindow>{window}](auto) {
                               if (w)
                                   w->close();
                               backend->destroy();
                           })
                           ->size({CDynamicSize::HT_SIZE_AUTO, CDynamicSize::HT_SIZE_AUTO, {1, 1}})
                           ->commence();

    null2->setGrow(true);

    layoutInner->addChild(title);
    layoutInner->addChild(hr);
    layoutInner->addChild(content);

    layout2->addChild(null2);
    layout2->addChild(donateButton);
    layout2->addChild(openNewsButton);
    layout2->addChild(closeButton);

    layout->addChild(layout2);

    window->m_events.closeRequest.listenStatic([w = WP<IWindow>{window}] {
        w->close();
        backend->destroy();
    });

    window->open();

    backend->enterLoop();

    return 0;
}