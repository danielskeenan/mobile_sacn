import {t} from "i18next";

/**
 * Set the page title, prefixing with the site name.
 * @param title The page title, or a list of title components.  If empty. only the app name will be used.
 */
export default function setPageTitle(title: string | string[] | null = null) {
    if (title === null) {
        title = [];
    } else if (!Array.isArray(title)) {
        title = [title];
    }
    title.unshift(t("app"));
    document.title = title.join(" - ");
}
