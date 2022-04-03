export default function stateObjectReducer<T extends Record<string, any>>(currentState: T, newState: Partial<T>) {
    return {...currentState, ...newState};
}
