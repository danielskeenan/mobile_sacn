module.exports = {
    webpack: function (config, env) {
        // Tell webpack about source assets.
        // See https://webpack.js.org/guides/asset-modules/#source-assets
        config["module"]["rules"].push({
            test: /(LICENSE|\.txt|\.md)$/i,
            type: "asset/source",
        });

        return config;
    },
};
