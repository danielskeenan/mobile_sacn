import 'jest-extended';

declare module '*/LICENSE' {
    const content: string;
    export default content;
}
