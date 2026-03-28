import {LevelDisplayMode} from "@/common/levelDisplay";
import {createEventListener} from "@solid-primitives/event-listener";
import {createContext, createEffect, ParentComponent, useContext} from "solid-js";
import {createStore, SetStoreFunction, Store} from "solid-js/store";

export enum ColorScheme {
    Light = "light",
    Dark = "dark",
}

export interface IAppContext {
    wsRoot: string;
    preferredColorScheme?: ColorScheme;
    activeColorScheme: ColorScheme;
    levelDisplayMode: LevelDisplayMode;
}

const defaultAppContext: IAppContext = {
    wsRoot: `${document.location.origin}/ws`,
    preferredColorScheme: undefined,
    activeColorScheme: ColorScheme.Light,
    levelDisplayMode: LevelDisplayMode.PERCENT,
};

const AppContext = createContext<[Store<IAppContext>, SetStoreFunction<IAppContext>]>();
export const useAppContext = () => useContext(AppContext)!;

export const AppContextProvider: ParentComponent = (props) => {
    const [appStore, setAppStore] = createStore<IAppContext>(defaultAppContext);

    // Color mode updates
    createEffect(() => {
        // Update Bootstrap color mode.
        document.documentElement.dataset.bsTheme = appStore.activeColorScheme;
    });
    const prefersDarkMode = window.matchMedia("(prefers-color-scheme: dark)");
    createEventListener(prefersDarkMode, "change", e => {
        if (appStore.preferredColorScheme === undefined) {
            setAppStore("activeColorScheme", e.matches ? ColorScheme.Dark : ColorScheme.Light);
        }
    });
    createEffect(()=>{
        let activeColorScheme = appStore.preferredColorScheme;
        if (activeColorScheme === undefined) {
            activeColorScheme = prefersDarkMode.matches ? ColorScheme.Dark : ColorScheme.Light;
        }
        setAppStore("activeColorScheme", activeColorScheme);
    })

    return (
        <AppContext.Provider value={[appStore, setAppStore]}>
            {props.children}
        </AppContext.Provider>
    );
};
