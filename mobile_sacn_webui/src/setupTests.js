import * as jestExtendedMatchers from 'jest-extended';

// add all jest-extended matchers
expect.extend(jestExtendedMatchers);

const localStorageMock = {
    getItem: jest.fn(),
    setItem: jest.fn(),
    removeItem: jest.fn(),
    clear: jest.fn(),
};
global.localStorage = localStorageMock;
