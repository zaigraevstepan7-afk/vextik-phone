#ifndef A_NATIVE_WINDOW_CREATOR_H // !A_NATIVE_WINDOW_CREATOR_H
#define A_NATIVE_WINDOW_CREATOR_H
 
#include <android/native_window.h>
#include <dlfcn.h>
#include <sys/system_properties.h>

#include <cstddef>
#include <unordered_map>
#include <string>
#include <vector>
 
#define ResolveMethod(ClassName, MethodName, Handle, MethodSignature)                                                                    \
    ClassName##__##MethodName = reinterpret_cast<decltype(ClassName##__##MethodName)>(symbolMethod.Find(Handle, MethodSignature));       \
    if (nullptr == ClassName##__##MethodName)                                                                                            \
    {                                                                                                                                    \
     \
    }

namespace android {
    namespace detail {
        namespace ui {
            struct LayerStack {
                uint32_t id = UINT32_MAX;
            };

            enum class Rotation {
                Rotation0 = 0,
                Rotation90 = 1,
                Rotation180 = 2,
                Rotation270 = 3
            };

            struct Size {
                int32_t width = -1;
                int32_t height = -1;
            };

            struct DisplayState {
                LayerStack layerStack;
                Rotation orientation = Rotation::Rotation0;
                Size layerStackSpaceRect;
            };

            typedef int64_t nsecs_t; 
            struct DisplayInfo {
                uint32_t w{0};
                uint32_t h{0};
                float xdpi{0};
                float ydpi{0};
                float fps{0};
                float density{0};
                uint8_t orientation{0};
                bool secure{false};
                nsecs_t appVsyncOffset{0};
                nsecs_t presentationDeadline{0};
                uint32_t viewportW{0};
                uint32_t viewportH{0};
            };

            enum class DisplayType {
                DisplayIdMain = 0,
                DisplayIdHdmi = 1
            };

            struct PhysicalDisplayId {
                uint64_t value;
            };
        }

        struct String8;

        struct LayerMetadata;

        struct Surface;

        struct SurfaceControl;

        struct SurfaceComposerClientTransaction;

        struct SurfaceComposerClient;

        template<typename any_t>
        struct StrongPointer {
            union {
                any_t *pointer;
                char padding[sizeof(std::max_align_t)];
            };

            inline any_t *operator->() const { return pointer; }

            inline any_t *get() const { return pointer; }

            inline explicit operator bool() const { return nullptr != pointer; }
        };

        struct Functionals {
            struct SymbolMethod {
                void *(*Open)(const char *filename, int flag) = nullptr;

                void *(*Find)(void *handle, const char *symbol) = nullptr;

                int (*Close)(void *handle) = nullptr;
            };

            int32_t systemVersion = 0;
            int32_t sdkVersion = 0;

            void (*RefBase__IncStrong)(void *thiz, void *id) = nullptr;

            void (*RefBase__DecStrong)(void *thiz, void *id) = nullptr;

            void (*String8__Constructor)(void *thiz, const char *const data) = nullptr;

            void (*String8__Destructor)(void *thiz) = nullptr;

            void (*LayerMetadata__Constructor)(void *thiz) = nullptr;

            void (*LayerMetadata__setInt32)(void *thiz, uint32_t type, int32_t value) = nullptr;

            void (*SurfaceComposerClient__Constructor)(void *thiz) = nullptr;

            void (*SurfaceComposerClient__Destructor)(void *thiz) = nullptr;

            StrongPointer<void>
            (*SurfaceComposerClient__CreateSurface)(void *thiz, void *name, uint32_t w, uint32_t h, int32_t format,
                                                    uint32_t flags, void *parentHandle, void *layerMetadata,
                                                    uint32_t *outTransformHint) = nullptr;

            StrongPointer<void> (*SurfaceComposerClient__GetInternalDisplayToken)() = nullptr;

            StrongPointer<void> (*SurfaceComposerClient__GetBuiltInDisplay)(ui::DisplayType type) = nullptr;

            int32_t (*SurfaceComposerClient__GetDisplayState)(StrongPointer<void> &display,
                                                              ui::DisplayState *displayState) = nullptr;

            int32_t (*SurfaceComposerClient__GetDisplayInfo)(StrongPointer<void> &display,
                                                             ui::DisplayInfo *displayInfo) = nullptr;

            std::vector<ui::PhysicalDisplayId> (*SurfaceComposerClient__GetPhysicalDisplayIds)() = nullptr;

            StrongPointer<void>
            (*SurfaceComposerClient__GetPhysicalDisplayToken)(ui::PhysicalDisplayId displayId) = nullptr;

            void (*SurfaceComposerClient__Transaction__Constructor)(void *thiz) = nullptr;

            void *(*SurfaceComposerClient__Transaction__SetLayer)(void *thiz, StrongPointer<void> &surfaceControl,
                                                                  int32_t z) = nullptr;

            void *(*SurfaceComposerClient__Transaction__SetBackgroundBlurRadius)(void *thiz, StrongPointer<void> &surfaceControl,
                                                                          int32_t radius) = nullptr;

            void *(*SurfaceComposerClient__Transaction__SetFlags)(void *thiz, StrongPointer<void> &surfaceControl,
                                                                 uint32_t flags, uint32_t mask) = nullptr;

            int32_t (*SurfaceComposerClient__Transaction__Apply)(void *thiz, bool synchronous, bool oneWay) = nullptr;

            int32_t (*SurfaceControl__Validate)(void *thiz) = nullptr;

            StrongPointer<Surface> (*SurfaceControl__GetSurface)(void *thiz) = nullptr;

            void (*SurfaceControl__DisConnect)(void *thiz) = nullptr;

            void* (*ASurfaceTransaction_create)() = nullptr;
            void (*ASurfaceTransaction_apply)(void* transaction) = nullptr;
            void (*ASurfaceTransaction_delete)(void* transaction) = nullptr;
            void (*ASurfaceTransaction_setFlags)(void* transaction, void* surface_control, uint32_t flags, uint32_t mask) = nullptr;
            void (*ASurfaceTransaction_setBackgroundBlurRadius)(void* transaction, void* surface_control, int32_t radius) = nullptr;

            Functionals(const SymbolMethod &symbolMethod) {
                char prop_value[128] = {0};
                
                __system_property_get("ro.build.version.release", prop_value);
                if (prop_value[0] != 0) {
                    try { systemVersion = std::stoi(prop_value); } catch (...) { systemVersion = 0; }
                }

                memset(prop_value, 0, sizeof(prop_value));
                __system_property_get("ro.build.version.sdk", prop_value);
                if (prop_value[0] != 0) {
                    try { sdkVersion = std::stoi(prop_value); } catch (...) { sdkVersion = 0; }
                }

                if (sdkVersion < 26 && systemVersion < 8) {
                    exit(0);
                    return;
                }

                static std::unordered_map<size_t, std::unordered_map<void **, const char *>> patchesTable = {
                        {
                                16,
                                {
                                        {reinterpret_cast<void **>(&LayerMetadata__Constructor),               "_ZN7android3gui13LayerMetadataC2Ev"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface),     "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEENS_3gui13LayerMetadataEPj"},
                                        {reinterpret_cast<void **>(&LayerMetadata__setInt32),                  "_ZN7android3gui13LayerMetadata8setInt32Eji"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__Transaction__Apply), "_ZN7android21SurfaceComposerClient11Transaction5applyEb"},
                                },
                                },
                        {
                                15,
                                {
                                        {reinterpret_cast<void **>(&LayerMetadata__Constructor),               "_ZN7android3gui13LayerMetadataC2Ev"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface),     "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEENS_3gui13LayerMetadataEPj"},
                                        {reinterpret_cast<void **>(&LayerMetadata__setInt32),                  "_ZN7android3gui13LayerMetadata8setInt32Eji"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__Transaction__Apply), "_ZN7android21SurfaceComposerClient11Transaction5applyEb"},
                                },
                        },
                        {
                                14,
                                {
                                        {reinterpret_cast<void **>(&LayerMetadata__Constructor),                "_ZN7android3gui13LayerMetadataC2Ev"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface),     "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEENS_3gui13LayerMetadataEPj"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__Transaction__Apply), "_ZN7android21SurfaceComposerClient11Transaction5applyEb"},
                                },
                        },
                        {
                                13,
                                {
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__Transaction__Apply), "_ZN7android21SurfaceComposerClient11Transaction5applyEb"},
                                },
                        },
                        {
                                12,
                                {
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__Transaction__Apply), "_ZN7android21SurfaceComposerClient11Transaction5applyEb"},
                                },
                        },
                        {
                                11,
                                {
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface),      "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataEPj"},
                                        {reinterpret_cast<void **>(&SurfaceControl__GetSurface),               "_ZNK7android14SurfaceControl10getSurfaceEv"},
                                },
                        },
                        {
                                10,
                                {
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface),      "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlENS_13LayerMetadataE"},
                                        {reinterpret_cast<void **>(&SurfaceControl__GetSurface),               "_ZNK7android14SurfaceControl10getSurfaceEv"},
                                },
                        },
                        {
                                9,
                                {

                                        {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface),      "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEii"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__GetBuiltInDisplay), "_ZN7android21SurfaceComposerClient17getBuiltInDisplayEi"},
                                        {reinterpret_cast<void **>(&SurfaceControl__GetSurface), "_ZNK7android14SurfaceControl10getSurfaceEv"},
                                },
                        },
                        {
                                8,
                                {

                                        {reinterpret_cast<void **>(&SurfaceComposerClient__CreateSurface),      "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijPNS_14SurfaceControlEjj"},
                                        {reinterpret_cast<void **>(&SurfaceComposerClient__GetBuiltInDisplay), "_ZN7android21SurfaceComposerClient17getBuiltInDisplayEi"},
                                        {reinterpret_cast<void **>(&SurfaceControl__GetSurface), "_ZNK7android14SurfaceControl10getSurfaceEv"},
                                },
                        },
                };

#ifdef __LP64__
                auto libgui = symbolMethod.Open("/system/lib64/libgui.so", RTLD_LAZY);
                auto libutils = symbolMethod.Open("/system/lib64/libutils.so", RTLD_LAZY);
                auto libandroid = symbolMethod.Open("/system/lib64/libandroid.so", RTLD_LAZY);
#else
                auto libgui = symbolMethod.Open("/system/lib/libgui.so", RTLD_LAZY);
                auto libutils = symbolMethod.Open("/system/lib/libutils.so", RTLD_LAZY);
                auto libandroid = symbolMethod.Open("/system/lib/libandroid.so", RTLD_LAZY);
#endif

               
                if (libandroid) {
                    ASurfaceTransaction_create = (void* (*)())symbolMethod.Find(libandroid, "ASurfaceTransaction_create");
                    ASurfaceTransaction_apply = (void (*)(void*))symbolMethod.Find(libandroid, "ASurfaceTransaction_apply");
                    ASurfaceTransaction_delete = (void (*)(void*))symbolMethod.Find(libandroid, "ASurfaceTransaction_delete");
                    ASurfaceTransaction_setFlags = (void (*)(void*, void*, uint32_t, uint32_t))symbolMethod.Find(libandroid, "ASurfaceTransaction_setFlags");
                    ASurfaceTransaction_setBackgroundBlurRadius = (void (*)(void*, void*, int32_t))symbolMethod.Find(libandroid, "ASurfaceTransaction_setBackgroundBlurRadius");
                }

                ResolveMethod(RefBase, IncStrong, libutils, "_ZNK7android7RefBase9incStrongEPKv");
                ResolveMethod(RefBase, DecStrong, libutils, "_ZNK7android7RefBase9decStrongEPKv");

                ResolveMethod(String8, Constructor, libutils, "_ZN7android7String8C2EPKc");
                ResolveMethod(String8, Destructor, libutils, "_ZN7android7String8D2Ev");

                ResolveMethod(LayerMetadata, Constructor, libgui, "_ZN7android13LayerMetadataC2Ev");
                ResolveMethod(LayerMetadata, setInt32, libgui, "_ZN7android13LayerMetadata8setInt32Eji");

                ResolveMethod(SurfaceComposerClient, Constructor, libgui, "_ZN7android21SurfaceComposerClientC2Ev");
                ResolveMethod(SurfaceComposerClient, CreateSurface, libgui,
                              "_ZN7android21SurfaceComposerClient13createSurfaceERKNS_7String8EjjijRKNS_2spINS_7IBinderEEENS_13LayerMetadataEPj");
                ResolveMethod(SurfaceComposerClient, GetInternalDisplayToken, libgui,
                              "_ZN7android21SurfaceComposerClient23getInternalDisplayTokenEv");
                ResolveMethod(SurfaceComposerClient, GetDisplayState, libgui,
                              "_ZN7android21SurfaceComposerClient15getDisplayStateERKNS_2spINS_7IBinderEEEPNS_2ui12DisplayStateE");
                ResolveMethod(SurfaceComposerClient, GetDisplayInfo, libgui,
                              "_ZN7android21SurfaceComposerClient14getDisplayInfoERKNS_2spINS_7IBinderEEEPNS_11DisplayInfoE");
                ResolveMethod(SurfaceComposerClient, GetPhysicalDisplayIds, libgui,
                              "_ZN7android21SurfaceComposerClient21getPhysicalDisplayIdsEv");
                ResolveMethod(SurfaceComposerClient, GetPhysicalDisplayToken, libgui,
                              "_ZN7android21SurfaceComposerClient23getPhysicalDisplayTokenENS_17PhysicalDisplayIdE");

                ResolveMethod(SurfaceComposerClient__Transaction, Constructor, libgui,
                              "_ZN7android21SurfaceComposerClient11TransactionC2Ev");
                ResolveMethod(SurfaceComposerClient__Transaction, SetLayer, libgui,
                              "_ZN7android21SurfaceComposerClient11Transaction8setLayerERKNS_2spINS_14SurfaceControlEEEi");
                
                ResolveMethod(SurfaceComposerClient__Transaction, SetFlags, libgui,
                              "_ZN7android21SurfaceComposerClient11Transaction8setFlagsERKNS_2spINS_14SurfaceControlEEjj");

                const char* blur_symbols[] = {
                    "_ZN7android21SurfaceComposerClient11Transaction22setBackgroundBlurRadiusERKNS_2spINS_14SurfaceControlEEEi", 
                    "_ZN7android21SurfaceComposerClient11Transaction22setBackgroundBlurRadiusERKNS_2spINS_14SurfaceControlEEEj", 
                    "_ZN7android3gui21SurfaceComposerClient11Transaction22setBackgroundBlurRadiusERKNS_2spINS_14SurfaceControlEEEi", 
                    "_ZN7android3gui21SurfaceComposerClient11Transaction22setBackgroundBlurRadiusERKNS_2spINS0_14SurfaceControlEEEi", 
                    "_ZN7android3gui21SurfaceComposerClient11Transaction22setBackgroundBlurRadiusERKNS_2spINS0_14SurfaceControlEEEj", 
                    "_ZN7android21SurfaceComposerClient11Transaction22setBackgroundBlurRadiusERKNS_2spINS_14SurfaceControlEEEf"  
                };

                for (const char* sym : blur_symbols) {
                    SurfaceComposerClient__Transaction__SetBackgroundBlurRadius = 
                        reinterpret_cast<void *(*)(void *, StrongPointer<void> &, int32_t)>(symbolMethod.Find(libgui, sym));
                    if (SurfaceComposerClient__Transaction__SetBackgroundBlurRadius != nullptr) break;
                }
                
                ResolveMethod(SurfaceComposerClient__Transaction, Apply, libgui,
                              "_ZN7android21SurfaceComposerClient11Transaction5applyEbb");
                
                
                if (SurfaceComposerClient__Transaction__Apply == nullptr) {
                    ResolveMethod(SurfaceComposerClient__Transaction, Apply, libgui,
                                  "_ZN7android21SurfaceComposerClient11Transaction5applyEb");
                }

                ResolveMethod(SurfaceControl, Validate, libgui, "_ZNK7android14SurfaceControl8validateEv");
                ResolveMethod(SurfaceControl, GetSurface, libgui, "_ZN7android14SurfaceControl10getSurfaceEv");
                ResolveMethod(SurfaceControl, DisConnect, libgui, "_ZN7android14SurfaceControl10disconnectEv");

                auto it = patchesTable.find(systemVersion);
                if (it != patchesTable.end()) {
                    for (const auto &[patchTo, signature] : patchesTable.at(systemVersion))
                    {
                        *patchTo = symbolMethod.Find(libgui, signature);
                        if (nullptr != *patchTo)
                            continue;
                    }
                }

                symbolMethod.Close(libutils);
                symbolMethod.Close(libgui);
            }
            
            static const Functionals &
            GetInstance(const SymbolMethod &symbolMethod = {.Open = dlopen, .Find = dlsym, .Close = dlclose}) {
                static Functionals functionals(symbolMethod);

                return functionals;
            }
        };

        struct String8 {
            char data[1024];

            String8(const char *const string) {
                Functionals::GetInstance().String8__Constructor(data, string);
            }

            ~String8() {
                Functionals::GetInstance().String8__Destructor(data);
            }

            operator void *() {
                return reinterpret_cast<void *>(data);
            }
        };

        struct LayerMetadata {
            char data[1024];

            LayerMetadata() {
                if (9 < Functionals::GetInstance().systemVersion)
                    Functionals::GetInstance().LayerMetadata__Constructor(data);
            }

            void setInt32(uint32_t type, int32_t value) {
                if (9 < Functionals::GetInstance().systemVersion)
                    Functionals::GetInstance().LayerMetadata__setInt32(data, type, value);
            }

            operator void *() {
                if (9 < Functionals::GetInstance().systemVersion)
                    return reinterpret_cast<void *>(data);
                else
                    return nullptr;
            }
        };

        struct Surface {
        };

        struct SurfaceControl {
            void *data;

            SurfaceControl() : data(nullptr) {}

            SurfaceControl(void *data) : data(data) {}

            int32_t Validate() {
                if (nullptr == data)
                    return 0;

                return Functionals::GetInstance().SurfaceControl__Validate(data);
            }

            Surface *GetSurface() {
                if (nullptr == data)
                    return nullptr;

                auto result = Functionals::GetInstance().SurfaceControl__GetSurface(data);

                return reinterpret_cast<Surface *>(reinterpret_cast<size_t>(result.pointer) +
                                                   sizeof(std::max_align_t) / 2);
            }

            void DisConnect() {
                if (nullptr == data)
                    return;

                Functionals::GetInstance().SurfaceControl__DisConnect(data);
            }

            void DestroySurface(Surface *surface) {
                if (nullptr == data || nullptr == surface)
                    return;

                Functionals::GetInstance().RefBase__DecStrong(
                        reinterpret_cast<Surface *>(reinterpret_cast<size_t>(surface) - sizeof(std::max_align_t) / 2),
                        this);
                DisConnect();
                Functionals::GetInstance().RefBase__DecStrong(data, this);
            }
        };

        struct SurfaceComposerClientTransaction {
            char data[1024];

            SurfaceComposerClientTransaction() {
                Functionals::GetInstance().SurfaceComposerClient__Transaction__Constructor(data);
            }

            void *SetLayer(StrongPointer<void> &surfaceControl, int32_t z) {
                return Functionals::GetInstance().SurfaceComposerClient__Transaction__SetLayer(data, surfaceControl, z);
            }

            void *SetBackgroundBlurRadius(StrongPointer<void> &surfaceControl, int32_t radius) {
                if (Functionals::GetInstance().SurfaceComposerClient__Transaction__SetBackgroundBlurRadius) {
                    return Functionals::GetInstance().SurfaceComposerClient__Transaction__SetBackgroundBlurRadius(data,
                                                                                                            surfaceControl,
                                                                                                            radius);
                }
                return nullptr;
            }

            void *SetFlags(StrongPointer<void> &surfaceControl, uint32_t flags, uint32_t mask) {
                if (Functionals::GetInstance().SurfaceComposerClient__Transaction__SetFlags) {
                    return Functionals::GetInstance().SurfaceComposerClient__Transaction__SetFlags(data, surfaceControl, flags, mask);
                }
                return nullptr;
            }

            int32_t Apply(bool synchronous, bool oneWay) {
                if (nullptr == Functionals::GetInstance().SurfaceComposerClient__Transaction__Apply)
                    return -1;

                if (Functionals::GetInstance().systemVersion >= 12) {
                    return reinterpret_cast<int32_t (*)(void *, bool)>(Functionals::GetInstance().SurfaceComposerClient__Transaction__Apply)(data, synchronous);
                } else {
                    return Functionals::GetInstance().SurfaceComposerClient__Transaction__Apply(data, synchronous, oneWay);
                }
            }
        };

        struct SurfaceComposerClient {
            char data[1024];

            SurfaceComposerClient() {
                Functionals::GetInstance().SurfaceComposerClient__Constructor(data);
                Functionals::GetInstance().RefBase__IncStrong(data, this);
            }

            SurfaceControl CreateSurface(const char *name, int32_t width, int32_t height, bool hide, bool secure) {
                void *parentHandle = nullptr;
                String8 windowName(name);
                LayerMetadata layerMetadata;
                uint32_t flags = secure ? 0x80 : 0;

                if (12 <= Functionals::GetInstance().systemVersion) {
                    static void *fakeParentHandleForBinder = nullptr;
                    parentHandle = &fakeParentHandleForBinder;
                }

                StrongPointer<void> result{};
                if (Functionals::GetInstance().systemVersion <= 9) {
                    int value = -1;
                    if (!secure && hide)
                        value = 441731;

                    result = reinterpret_cast<StrongPointer<void> (*)(void *, void *, uint32_t, uint32_t, int32_t,
                                                                      uint32_t, void *, int32_t, int32_t)>(
                            Functionals::GetInstance().SurfaceComposerClient__CreateSurface)(data, windowName, width,
                                                                                             height, 1, flags,
                                                                                             nullptr,
                                                                                             value, -1);
                } else {
                    if (!secure && hide) {
                        if (Functionals::GetInstance().systemVersion > 9 &&
                            Functionals::GetInstance().systemVersion < 12) {
                            layerMetadata.setInt32(2, 441731);
                        } else {
                            flags = 0x40;
                        }
                    }
                    result = Functionals::GetInstance().SurfaceComposerClient__CreateSurface(data, windowName, width,
                                                                                             height, 1, flags,
                                                                                             parentHandle,
                                                                                             layerMetadata, nullptr);
                }

                return {result.get()};
            }

            bool GetDisplayInfo(ui::DisplayState *displayInfo) {
                StrongPointer<void> defaultDisplay;

                if (9 >= Functionals::GetInstance().systemVersion)
                    defaultDisplay = Functionals::GetInstance().SurfaceComposerClient__GetBuiltInDisplay(
                            ui::DisplayType::DisplayIdMain);
                else {
                    if (14 > Functionals::GetInstance().systemVersion) {
                        defaultDisplay = Functionals::GetInstance().SurfaceComposerClient__GetInternalDisplayToken();
                    } else {
                        auto displayIds = Functionals::GetInstance().SurfaceComposerClient__GetPhysicalDisplayIds();
                        if (displayIds.empty())
                            return false;

                        defaultDisplay = Functionals::GetInstance().SurfaceComposerClient__GetPhysicalDisplayToken(
                                displayIds[0]);
                    }
                }

                if (nullptr == defaultDisplay.get())
                    return false;

                if (11 <= Functionals::GetInstance().systemVersion || Functionals::GetInstance().systemVersion > 14)
                    return 0 == Functionals::GetInstance().SurfaceComposerClient__GetDisplayState(defaultDisplay,
                                                                                                  displayInfo);
                else {
                    ui::DisplayInfo realDisplayInfo{};
                    if (0 != Functionals::GetInstance().SurfaceComposerClient__GetDisplayInfo(defaultDisplay,
                                                                                              &realDisplayInfo))
                        return false;

                    displayInfo->layerStackSpaceRect.width = realDisplayInfo.w;
                    displayInfo->layerStackSpaceRect.height = realDisplayInfo.h;
                    displayInfo->orientation = static_cast<ui::Rotation>(realDisplayInfo.orientation);

                    return true;
                }
            }
        };
    }

    class ANativeWindowCreator {
    public:
        struct DisplayInfo {
            uint32_t orientation;
            int32_t width;
            int32_t height;
        };

    public:
        static detail::SurfaceComposerClient &GetComposerInstance() {
            static detail::SurfaceComposerClient surfaceComposerClient;

            return surfaceComposerClient;
        }

        static DisplayInfo GetDisplayInfo() {
            auto &surfaceComposerClient = GetComposerInstance();
            detail::ui::DisplayState displayInfo{};

            if (!surfaceComposerClient.GetDisplayInfo(&displayInfo))
                return {};
                
            if (detail::Functionals::GetInstance().systemVersion <= 10) {
                DisplayInfo mDisplayInfo{};
                if (static_cast<uint32_t>(displayInfo.orientation) == 1 || static_cast<uint32_t>(displayInfo.orientation) == 3) {
                    mDisplayInfo.width = displayInfo.layerStackSpaceRect.height;
                    mDisplayInfo.height = displayInfo.layerStackSpaceRect.width;
                } else {
                    mDisplayInfo.width = displayInfo.layerStackSpaceRect.width;
                    mDisplayInfo.height = displayInfo.layerStackSpaceRect.height;
                }
                mDisplayInfo.orientation = static_cast<uint32_t>(displayInfo.orientation);
                return mDisplayInfo;
            }
            
            return DisplayInfo{
                    .orientation = static_cast<uint32_t>(displayInfo.orientation),
                    .width = displayInfo.layerStackSpaceRect.width,
                    .height = displayInfo.layerStackSpaceRect.height,
            };
        }

        static ANativeWindow *
        Create(const char *name, int32_t width = -1, int32_t height = -1, bool hide = true, bool secure = false) {
            auto &surfaceComposerClient = GetComposerInstance();

            while (-1 == width || -1 == height) {
                detail::ui::DisplayState displayInfo{};

                if (!surfaceComposerClient.GetDisplayInfo(&displayInfo))
                    break;
                    
                if (detail::Functionals::GetInstance().systemVersion <= 10) {
                    if (static_cast<uint32_t>(displayInfo.orientation) == 1 || static_cast<uint32_t>(displayInfo.orientation) == 3) {
                        width = displayInfo.layerStackSpaceRect.height;
                        height = displayInfo.layerStackSpaceRect.width;
                    } else {
                        width = displayInfo.layerStackSpaceRect.width;
                        height = displayInfo.layerStackSpaceRect.height;
                    }
                } else {
                    width = displayInfo.layerStackSpaceRect.width;
                    height = displayInfo.layerStackSpaceRect.height;
                }
                
                break;
            }

            auto surfaceControl = surfaceComposerClient.CreateSurface(name, width, height, hide, secure);
            auto nativeWindow = reinterpret_cast<ANativeWindow *>(surfaceControl.GetSurface());

            m_cachedSurfaceControl.emplace(nativeWindow, surfaceControl);
            return nativeWindow;
        }

        static int32_t GetSDKVersion() {
            return detail::Functionals::GetInstance().sdkVersion;
        }

        static bool SupportsBlur() {
            return (detail::Functionals::GetInstance().sdkVersion >= 31) && 
                    (detail::Functionals::GetInstance().ASurfaceTransaction_setBackgroundBlurRadius != nullptr || 
                     detail::Functionals::GetInstance().SurfaceComposerClient__Transaction__SetBackgroundBlurRadius != nullptr);
        }

        static bool IsBlurSupported() {
            return SupportsBlur();
        }

        static void SetBlurRadius(ANativeWindow *nativeWindow, int32_t radius) {
            auto it = m_cachedSurfaceControl.find(nativeWindow);
            if (it != m_cachedSurfaceControl.end()) {
                if (detail::Functionals::GetInstance().ASurfaceTransaction_setBackgroundBlurRadius && 
                    detail::Functionals::GetInstance().ASurfaceTransaction_create) {
                    
                    void* transaction = detail::Functionals::GetInstance().ASurfaceTransaction_create();
                    if (transaction) {
                        detail::Functionals::GetInstance().ASurfaceTransaction_setBackgroundBlurRadius(transaction, it->second.data, radius);
                        detail::Functionals::GetInstance().ASurfaceTransaction_apply(transaction);
                        detail::Functionals::GetInstance().ASurfaceTransaction_delete(transaction);
                        return;
                    }
                }

                if (detail::Functionals::GetInstance().SurfaceComposerClient__Transaction__SetBackgroundBlurRadius != nullptr) {
                    detail::SurfaceComposerClientTransaction transaction;
                    detail::StrongPointer<void> sc;
                    sc.pointer = it->second.data;
                    transaction.SetBackgroundBlurRadius(sc, radius);
                    transaction.Apply(false, true);
                }
            }
        }

        static void SetSecure(ANativeWindow *nativeWindow, bool secure) {
            auto it = m_cachedSurfaceControl.find(nativeWindow);
            if (it != m_cachedSurfaceControl.end()) {
                if (detail::Functionals::GetInstance().ASurfaceTransaction_setFlags && 
                    detail::Functionals::GetInstance().ASurfaceTransaction_create) {
                    
                    void* transaction = detail::Functionals::GetInstance().ASurfaceTransaction_create();
                    if (transaction) {
                        detail::Functionals::GetInstance().ASurfaceTransaction_setFlags(transaction, it->second.data, secure ? 0x1 : 0, 0x1);
                        detail::Functionals::GetInstance().ASurfaceTransaction_apply(transaction);
                        detail::Functionals::GetInstance().ASurfaceTransaction_delete(transaction);
                        return;
                    }
                }

                if (detail::Functionals::GetInstance().SurfaceComposerClient__Transaction__SetFlags != nullptr) {
                    detail::SurfaceComposerClientTransaction transaction;
                    detail::StrongPointer<void> sc;
                    sc.pointer = it->second.data;
                    transaction.SetFlags(sc, secure ? 0x80 : 0, 0x80);
                    transaction.Apply(false, true);
                }
            }
        }

        static void Destroy(ANativeWindow *nativeWindow) {
            auto it = m_cachedSurfaceControl.find(nativeWindow);
            if (it != m_cachedSurfaceControl.end())
                return;

            m_cachedSurfaceControl[nativeWindow].DestroySurface(reinterpret_cast<detail::Surface *>(nativeWindow));
            m_cachedSurfaceControl.erase(nativeWindow);
        }

    private:
        inline static std::unordered_map<ANativeWindow *, detail::SurfaceControl> m_cachedSurfaceControl;
    };
}

#undef ResolveMethod

#endif 
