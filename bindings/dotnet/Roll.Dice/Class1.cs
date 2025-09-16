using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Roll.Dice;

/// <summary>
/// Exception thrown by dice operations
/// </summary>
public class DiceException : Exception
{
    public DiceException(string message) : base(message) { }
    public DiceException(string message, Exception innerException) : base(message, innerException) { }
}

/// <summary>
/// .NET bindings for the Roll dice library
/// </summary>
public static class Dice
{
    private const string LibName = "dice";

    // P/Invoke declarations
    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void dice_init(uint seed);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern int dice_roll(int sides);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern int dice_roll_multiple(int count, int sides);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern int dice_roll_individual(int count, int sides, [Out] int[] results);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern int dice_roll_notation(byte[] dice_notation);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr dice_version();

    /// <summary>
    /// Initialize the dice library with a seed
    /// </summary>
    /// <param name="seed">Random seed (null for time-based seed)</param>
    public static void Init(uint? seed = null)
    {
        dice_init(seed ?? 0);
    }

    /// <summary>
    /// Get the library version
    /// </summary>
    /// <returns>Version string</returns>
    public static string Version()
    {
        var ptr = dice_version();
        return Marshal.PtrToStringAnsi(ptr) ?? "unknown";
    }

    /// <summary>
    /// Roll a single die
    /// </summary>
    /// <param name="sides">Number of sides on the die</param>
    /// <returns>Random value between 1 and sides (inclusive)</returns>
    /// <exception cref="DiceException">Thrown when sides &lt;= 0</exception>
    public static int Roll(int sides)
    {
        if (sides <= 0)
            throw new DiceException($"Invalid number of sides: {sides}");

        int result = dice_roll(sides);
        if (result == -1)
            throw new DiceException($"Invalid number of sides: {sides}");

        return result;
    }

    /// <summary>
    /// Roll multiple dice and return the sum
    /// </summary>
    /// <param name="count">Number of dice to roll</param>
    /// <param name="sides">Number of sides on each die</param>
    /// <returns>Sum of all dice rolls</returns>
    /// <exception cref="DiceException">Thrown when count &lt;= 0 or sides &lt;= 0</exception>
    public static int RollMultiple(int count, int sides)
    {
        if (count <= 0)
            throw new DiceException($"Invalid count: {count}");
        if (sides <= 0)
            throw new DiceException($"Invalid number of sides: {sides}");

        int result = dice_roll_multiple(count, sides);
        if (result == -1)
            throw new DiceException($"Invalid parameters: count={count}, sides={sides}");

        return result;
    }

    /// <summary>
    /// Roll multiple dice and return individual results
    /// </summary>
    /// <param name="count">Number of dice to roll</param>
    /// <param name="sides">Number of sides on each die</param>
    /// <returns>Tuple of (sum, array of individual results)</returns>
    /// <exception cref="DiceException">Thrown when count &lt;= 0 or sides &lt;= 0</exception>
    public static (int sum, int[] individual) RollIndividual(int count, int sides)
    {
        if (count <= 0)
            throw new DiceException($"Invalid count: {count}");
        if (sides <= 0)
            throw new DiceException($"Invalid number of sides: {sides}");

        int[] results = new int[count];
        int sum = dice_roll_individual(count, sides, results);
        
        if (sum == -1)
            throw new DiceException($"Invalid parameters: count={count}, sides={sides}");

        return (sum, results);
    }

    /// <summary>
    /// Roll dice using RPG notation
    /// </summary>
    /// <param name="notation">Dice notation like "3d6", "1d20+5", "2d8-1"</param>
    /// <returns>Result of the dice roll</returns>
    /// <exception cref="DiceException">Thrown when notation is invalid</exception>
    public static int RollNotation(string notation)
    {
        if (string.IsNullOrEmpty(notation))
            throw new DiceException("Notation cannot be null or empty");

        byte[] notationBytes = Encoding.UTF8.GetBytes(notation + "\0"); // Add null terminator
        int result = dice_roll_notation(notationBytes);
        
        if (result == -1)
            throw new DiceException($"Invalid dice notation: {notation}");

        return result;
    }
}
