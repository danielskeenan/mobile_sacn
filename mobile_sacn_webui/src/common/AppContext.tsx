import {LevelDisplayMode} from "@/common/levelDisplay";
import {createEventListener} from "@solid-primitives/event-listener";
import {createContext, createEffect, ParentComponent, useContext} from "solid-js";
import {createStore, SetStoreFunction, Store} from "solid-js/store";

export enum ColorMode {
    Auto = "",
    Light = "light",
    Dark = "dark",
}

interface IAppContext {
    colorMode: ColorMode;
    levelDisplayMode: LevelDisplayMode;
}

const defaultAppContext: IAppContext = {
    colorMode: ColorMode.Auto,
    levelDisplayMode: LevelDisplayMode.PERCENT,
};

const AppContext = createContext<[Store<IAppContext>, SetStoreFunction<IAppContext>]>();
export const useAppContext = () => useContext(AppContext)!;

export const AppContextProvider: ParentComponent = (props) => {
    const prefersDarkMode = window.matchMedia("(prefers-color-scheme: dark)");

    const [appStore, setAppStore] = createStore<IAppContext>(Object.assign({}, defaultAppContext, {colorMode: prefersDarkMode.matches ? ColorMode.Dark : ColorMode.Light}));

    // Color mode updates
    createEffect(() => {
        // Update Bootstrap color mode.
        document.documentElement.dataset.bsTheme = appStore.colorMode;
    });
    createEventListener(prefersDarkMode, "change", e => {
        setAppStore("colorMode", e.matches ? ColorMode.Dark : ColorMode.Light);
    });

    return (
        <AppContext.Provider value={[appStore, setAppStore]}>
            {props.children}
        </AppContext.Provider>
    );
};
