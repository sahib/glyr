#define GLYR_VERSION_MAJOR "0"
#define GLYR_VERSION_MINOR "8"
#define GLYR_VERSION_MICRO "9"

#define GLYR_VERSION_MAJOR_INT 0
#define GLYR_VERSION_MINOR_INT 8
#define GLYR_VERSION_MICRO_INT 9

/**
 * GLYR_CHECK_VERSION:
 * @X: Macro version.
 * @Y: Mino version.
 * @Z: Micro version,
 *
 * Version to check glyr's version.
 * Example:
 * <informalexample>
 * <programlisting>
 * #if GLYR_CHECK_VERSION(0,8,7)
 *      puts("Version is above 0.87!");
 * #endif
 * </programlisting>
 * </informalexample>
*/
#define GLYR_CHECK_VERSION(X,Y,Z) (X <= GLYR_VERSION_MAJOR_INT && Y <= GLYR_VERSION_MINOR_INT && Z <= GLYR_VERSION_MICRO_INT)

#define GLYR_VERSION_NAME  "Horny Hornet [beta]"
#define GLYR_DEBUG         TRUE
