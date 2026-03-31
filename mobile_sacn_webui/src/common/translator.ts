import i18next from "i18next";
import LanguageDetector from "i18next-browser-languagedetector";
import HttpApi from "i18next-http-backend";
import {DEV} from "solid-js";

export default async function createTranslator() {
    return i18next
        .use(HttpApi)
        .use(LanguageDetector)
        .init({
            fallbackLng: "en",
            debug: DEV !== undefined,
            ns: ["common"],
            defaultNS: "common",
        });
}
