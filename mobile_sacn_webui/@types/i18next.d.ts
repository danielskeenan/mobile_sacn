import "i18next";
import nsChannelCheck from "../public/locales/en/channelCheck.json";
import nsCommon from "../public/locales/en/common.json";
import nsReceiveLevels from "../public/locales/en/receiveLevels.json";

declare module "i18next" {
    interface CustomTypeOptions {
        defaultNS: "common";
        resources: {
            common: typeof nsCommon;
            channelCheck: typeof nsChannelCheck;
            receiveLevels: typeof nsReceiveLevels;
        };
    }
}
