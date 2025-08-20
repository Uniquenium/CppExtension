#include "UDFrameless.h"

#include "UDTools.h"
#include <QDateTime>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QScreen>
#include <optional>
#include <pybind11/pybind11.h>

#ifdef Q_OS_WIN

static DwmSetWindowAttributeFunc pDwmSetWindowAttribute = nullptr;
static DwmExtendFrameIntoClientAreaFunc pDwmExtendFrameIntoClientArea = nullptr;
static DwmIsCompositionEnabledFunc pDwmIsCompositionEnabled = nullptr;
static DwmEnableBlurBehindWindowFunc pDwmEnableBlurBehindWindow = nullptr;
static SetWindowCompositionAttributeFunc pSetWindowCompositionAttribute = nullptr;
static GetDpiForWindowFunc pGetDpiForWindow = nullptr;
static GetSystemMetricsForDpiFunc pGetSystemMetricsForDpi = nullptr;

static RTL_OSVERSIONINFOW GetRealOSVersionImpl()
{
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    using RtlGetVersionPtr = NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW);
    auto pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(hMod, "RtlGetVersion"));
    RTL_OSVERSIONINFOW rovi {};
    rovi.dwOSVersionInfoSize = sizeof(rovi);
    pRtlGetVersion(&rovi);
    return rovi;
}

RTL_OSVERSIONINFOW GetRealOSVersion()
{
    static const auto result = GetRealOSVersionImpl();
    return result;
}

static inline bool isWin8OrGreater()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return (rovi.dwMajorVersion > 6) || (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion >= 2);
}

static inline bool isWin8Point1OrGreater()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return (rovi.dwMajorVersion > 6) || (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion >= 3);
}

static inline bool isWin10OrGreater()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return (rovi.dwMajorVersion > 10) || (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion >= 0);
}

static inline bool isWin101809OrGreater()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return (rovi.dwMajorVersion > 10) || (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion >= 0 && rovi.dwBuildNumber >= 17763);
}

static inline bool isWin101903OrGreater()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return (rovi.dwMajorVersion > 10) || (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion >= 0 && rovi.dwBuildNumber >= 18362);
}

static inline bool isWin11OrGreater()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return (rovi.dwMajorVersion > 10) || (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion >= 0 && rovi.dwBuildNumber >= 22000);
}

static inline bool isWin1122H2OrGreater()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return (rovi.dwMajorVersion > 10) || (rovi.dwMajorVersion == 10 && rovi.dwMinorVersion >= 0 && rovi.dwBuildNumber >= 22621);
}

static inline bool isWin10Only()
{
    return isWin10OrGreater() && !isWin11OrGreater();
}

static inline bool isWin7Only()
{
    RTL_OSVERSIONINFOW rovi = GetRealOSVersion();
    return rovi.dwMajorVersion == 7;
}

static inline QByteArray qtNativeEventType()
{
    static const auto result = "windows_generic_MSG";
    return result;
}

static inline bool initializeFunctionPointers()
{
    HMODULE module = LoadLibraryW(L"dwmapi.dll");
    if (module) {
        if (!pDwmSetWindowAttribute) {
            pDwmSetWindowAttribute = reinterpret_cast<DwmSetWindowAttributeFunc>(
                GetProcAddress(module, "DwmSetWindowAttribute"));
            if (!pDwmSetWindowAttribute) {
                return false;
            }
        }
        if (!pDwmExtendFrameIntoClientArea) {
            pDwmExtendFrameIntoClientArea = reinterpret_cast<DwmExtendFrameIntoClientAreaFunc>(
                GetProcAddress(module, "DwmExtendFrameIntoClientArea"));
            if (!pDwmExtendFrameIntoClientArea) {
                return false;
            }
        }
        if (!pDwmIsCompositionEnabled) {
            pDwmIsCompositionEnabled = reinterpret_cast<DwmIsCompositionEnabledFunc>(
                ::GetProcAddress(module, "DwmIsCompositionEnabled"));
            if (!pDwmIsCompositionEnabled) {
                return false;
            }
        }
        if (!pDwmEnableBlurBehindWindow) {
            pDwmEnableBlurBehindWindow = reinterpret_cast<DwmEnableBlurBehindWindowFunc>(
                GetProcAddress(module, "DwmEnableBlurBehindWindow"));
            if (!pDwmEnableBlurBehindWindow) {
                return false;
            }
        }
    }

    HMODULE user32 = LoadLibraryW(L"user32.dll");
    if (module) {
        if (!pSetWindowCompositionAttribute) {
            pSetWindowCompositionAttribute = reinterpret_cast<SetWindowCompositionAttributeFunc>(
                GetProcAddress(user32, "SetWindowCompositionAttribute"));
            if (!pSetWindowCompositionAttribute) {
                return false;
            }
        }

        if (!pGetDpiForWindow) {
            pGetDpiForWindow = reinterpret_cast<GetDpiForWindowFunc>(GetProcAddress(user32, "GetDpiForWindow"));
            if (!pGetDpiForWindow) {
                return false;
            }
        }

        if (!pGetSystemMetricsForDpi) {
            pGetSystemMetricsForDpi = reinterpret_cast<GetSystemMetricsForDpiFunc>(
                GetProcAddress(user32, "GetSystemMetricsForDpi"));
            if (!pGetSystemMetricsForDpi) {
                return false;
            }
        }
    }

    return true;
}

static inline bool isCompositionEnabled()
{
    if (initializeFunctionPointers()) {
        BOOL composition_enabled = false;
        pDwmIsCompositionEnabled(&composition_enabled);
        return composition_enabled;
    }
    return false;
}

static inline void setShadow(HWND hwnd)
{
    const MARGINS shadow = { 1, 0, 0, 0 };
    if (initializeFunctionPointers()) {
        pDwmExtendFrameIntoClientArea(hwnd, &shadow);
    }
    if (isWin7Only()) {
        SetClassLong(hwnd, GCL_STYLE, GetClassLong(hwnd, GCL_STYLE) | CS_DROPSHADOW);
    }
}

static inline bool setWindowDarkMode(HWND hwnd, const BOOL enable)
{
    if (!initializeFunctionPointers()) {
        return false;
    }
    return bool(pDwmSetWindowAttribute(hwnd, 20, &enable, sizeof(BOOL)));
}

static inline std::optional<MONITORINFOEXW> getMonitorForWindow(const HWND hwnd)
{
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return std::nullopt;
    }
    const HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (!monitor) {
        return std::nullopt;
    }
    MONITORINFOEXW monitorInfo;
    ::SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (::GetMonitorInfoW(monitor, &monitorInfo) == FALSE) {
        return std::nullopt;
    }
    return monitorInfo;
};

static inline bool isFullScreen(const HWND hwnd)
{
    RECT windowRect = {};
    if (::GetWindowRect(hwnd, &windowRect) == FALSE) {
        return false;
    }
    const std::optional<MONITORINFOEXW> mi = getMonitorForWindow(hwnd);
    if (!mi.has_value()) {
        return false;
    }
    RECT rcMonitor = mi.value().rcMonitor;
    return windowRect.top == rcMonitor.top && windowRect.left == rcMonitor.left && windowRect.right == rcMonitor.right && windowRect.bottom == rcMonitor.bottom;
}

static inline bool isMaximized(const HWND hwnd)
{
    WINDOWPLACEMENT wp;
    ::GetWindowPlacement(hwnd, &wp);
    return wp.showCmd == SW_MAXIMIZE;
}

static inline quint32 getDpiForWindow(const HWND hwnd, const bool horizontal)
{
    if (const UINT dpi = pGetDpiForWindow(hwnd)) {
        return dpi;
    }
    if (const HDC hdc = ::GetDC(hwnd)) {
        bool valid = false;
        const int dpiX = ::GetDeviceCaps(hdc, LOGPIXELSX);
        const int dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
        if ((dpiX > 0) && (dpiY > 0)) {
            valid = true;
        }
        ::ReleaseDC(hwnd, hdc);
        if (valid) {
            return (horizontal ? dpiX : dpiY);
        }
    }
    return 96;
}

static inline int getSystemMetrics(const HWND hwnd, const int index, const bool horizontal)
{
    const UINT dpi = getDpiForWindow(hwnd, horizontal);
    if (const int result = pGetSystemMetricsForDpi(index, dpi); result > 0) {
        return result;
    }
    return ::GetSystemMetrics(index);
}

static inline quint32 getResizeBorderThickness(const HWND hwnd, const bool horizontal,
    const qreal devicePixelRatio)
{
    auto frame = horizontal ? SM_CXSIZEFRAME : SM_CYSIZEFRAME;
    auto result = getSystemMetrics(hwnd, frame, horizontal) + getSystemMetrics(hwnd, 92, horizontal);
    if (result > 0) {
        return result;
    }
    int thickness = isCompositionEnabled() ? 8 : 4;
    return qRound(thickness * devicePixelRatio);
}

static inline bool setWindowEffect(HWND hwnd, const QString& key, const bool& enable)
{
    static constexpr const MARGINS extendedMargins = { -1, -1, -1, -1 };
    if (key == QStringLiteral("mica")) {
        if (!isWin11OrGreater() || !initializeFunctionPointers()) {
            return false;
        }
        if (enable) {
            pDwmExtendFrameIntoClientArea(hwnd, &extendedMargins);
            if (isWin1122H2OrGreater()) {
                const DWORD backdropType = _DWMSBT_MAINWINDOW;
                pDwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
            } else {
                const BOOL enable = TRUE;
                pDwmSetWindowAttribute(hwnd, 1029, &enable, sizeof(enable));
            }
        } else {
            if (isWin1122H2OrGreater()) {
                const DWORD backdropType = _DWMSBT_AUTO;
                pDwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
            } else {
                const BOOL enable = FALSE;
                pDwmSetWindowAttribute(hwnd, 1029, &enable, sizeof(enable));
            }
        }
        return true;
    }

    if (key == QStringLiteral("mica-alt")) {
        if (!isWin1122H2OrGreater() || !initializeFunctionPointers()) {
            return false;
        }
        if (enable) {
            pDwmExtendFrameIntoClientArea(hwnd, &extendedMargins);
            const DWORD backdropType = _DWMSBT_TABBEDWINDOW;
            pDwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
        } else {
            const DWORD backdropType = _DWMSBT_AUTO;
            pDwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
        }
        return true;
    }

    if (key == QStringLiteral("acrylic")) {
        if (!isWin11OrGreater() || !initializeFunctionPointers()) {
            return false;
        }
        if (enable) {
            pDwmExtendFrameIntoClientArea(hwnd, &extendedMargins);
            DWORD system_backdrop_type = _DWMSBT_TRANSIENTWINDOW;
            pDwmSetWindowAttribute(hwnd, 38, &system_backdrop_type, sizeof(DWORD));
        } else {
            const DWORD backdropType = _DWMSBT_AUTO;
            pDwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
        }
        return true;
    }

    if (key == QStringLiteral("dwm-blur")) {
        if ((isWin7Only() && !isCompositionEnabled()) || !initializeFunctionPointers()) {
            return false;
        }
        if (enable) {
            if (isWin8OrGreater()) {
                ACCENT_POLICY policy {};
                policy.dwAccentState = ACCENT_ENABLE_BLURBEHIND;
                policy.dwAccentFlags = ACCENT_NONE;
                WINDOWCOMPOSITIONATTRIBDATA wcad {};
                wcad.Attrib = WCA_ACCENT_POLICY;
                wcad.pvData = &policy;
                wcad.cbData = sizeof(policy);
                pSetWindowCompositionAttribute(hwnd, &wcad);
            } else {
                DWM_BLURBEHIND bb {};
                bb.fEnable = TRUE;
                bb.dwFlags = DWM_BB_ENABLE;
                pDwmEnableBlurBehindWindow(hwnd, &bb);
            }
        } else {
            if (isWin8OrGreater()) {
                ACCENT_POLICY policy {};
                policy.dwAccentState = ACCENT_DISABLED;
                policy.dwAccentFlags = ACCENT_NONE;
                WINDOWCOMPOSITIONATTRIBDATA wcad {};
                wcad.Attrib = WCA_ACCENT_POLICY;
                wcad.pvData = &policy;
                wcad.cbData = sizeof(policy);
                pSetWindowCompositionAttribute(hwnd, &wcad);
            } else {
                DWM_BLURBEHIND bb {};
                bb.fEnable = FALSE;
                bb.dwFlags = DWM_BB_ENABLE;
                pDwmEnableBlurBehindWindow(hwnd, &bb);
            }
        }
        return true;
    }
    return false;
}

#endif

bool containsCursorToItem(QQuickItem* item)
{
    if (!item || !item->isVisible()) {
        return false;
    }
    auto point = item->window()->mapFromGlobal(QCursor::pos());
    auto rect = QRectF(item->mapToItem(item->window()->contentItem(), QPointF(0, 0)), item->size());
    if (rect.contains(point)) {
        return true;
    }
    return false;
}



    // HWND hwnd = reinterpret_cast<HWND>(window()->winId());
    // if (isWin11OrGreater()) {
    //     availableEffects({ "mica", "mica-alt", "acrylic", "dwm-blur", "normal" });
    // } else {
    //     availableEffects({ "dwm-blur", "normal" });
    // }
    // if (!_effect.isEmpty() && _useSystemEffect) {
    //     effective(setWindowEffect(hwnd, _effect, true));
    //     if (effective()) {
    //         _currentEffect = effect();
    //     }
    // }
    // connect(this, &LingmoFrameless::effectChanged, this, [hwnd, this] {
    //     if (effect() == _currentEffect) {
    //         return;
    //     }
    //     if (effective()) {
    //         setWindowEffect(hwnd, _currentEffect, false);
    //     }
    //     effective(setWindowEffect(hwnd, effect(), true));
    //     if (effective()) {
    //         _currentEffect = effect();
    //         _useSystemEffect = true;
    //     } else {
    //         _effect = "normal";
    //         _currentEffect = "normal";
    //         _useSystemEffect = false;
    //     }
    // });
    // connect(this, &LingmoFrameless::useSystemEffectChanged, this, [this] {
    //     if (!_useSystemEffect) {
    //         effect("normal");
    //     }
    // });
    // connect(this, &LingmoFrameless::isDarkModeChanged, this, [hwnd, this] {
    //     if (effective() && !_currentEffect.isEmpty() && _currentEffect != "normal") {
    //         setWindowDarkMode(hwnd, _isDarkMode);
    //     }
    // });

PYBIND11_MODULE(UDFrameless,m){
    m.def("setWindowEffect",&setWindowEffect,py::arg("hwnd"),py::arg("key"),py::arg("enable"));
}