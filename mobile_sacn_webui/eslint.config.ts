import stylistic from "@stylistic/eslint-plugin";

export default [
    {
        files: ["**/*.{js,mjs,cjs,ts,mts,cts}"],
        plugins: {
            "@stylistic": stylistic,
        },
    },
];
