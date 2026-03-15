/**
 * Get a theme color from the CSS variable.
 *
 * @see https://getbootstrap.com/docs/5.3/customize/color/
 *
 * @param name
 */
export default function getBootstrapColor(name: string): string {
    return window.getComputedStyle(document.documentElement).getPropertyValue(`--bs-${name}`);
}
