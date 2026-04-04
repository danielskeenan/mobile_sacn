import {t} from "i18next";

export default function NotFound() {
    return (
        <section class="text-gray-700 p-8">
            <h1 class="text-2xl font-bold">{t("errors.404.title")}</h1>
            <p class="mt-4">{t("errors.404.text")}</p>
        </section>
    );
}
