import React from "react";

export function handleNumberFieldChange(e: React.ChangeEvent<HTMLInputElement>) {
    const newValue = parseInt(e.target.value);
    if (isNaN(newValue)) {
        return 0;
    }
    return newValue;
}
