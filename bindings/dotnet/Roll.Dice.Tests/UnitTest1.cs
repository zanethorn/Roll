using Roll.Dice;
using Xunit;
using Xunit.Abstractions;

namespace Roll.Dice.Tests;

public class DiceTests
{
    private readonly ITestOutputHelper _output;

    public DiceTests(ITestOutputHelper output)
    {
        _output = output;
    }

    [Fact]
    public void TestVersion()
    {
        var version = Dice.Version();
        Assert.NotNull(version);
        Assert.NotEmpty(version);
        _output.WriteLine($"Library version: {version}");
    }

    [Fact]
    public void TestInitialization()
    {
        // Test with seed
        Dice.Init(12345);
        
        // Test without seed
        Dice.Init();
        
        // Test with null seed
        Dice.Init(null);
    }

    [Fact]
    public void TestSingleRoll()
    {
        Dice.Init(12345);
        
        // Test valid rolls
        var result = Dice.Roll(6);
        Assert.InRange(result, 1, 6);
        
        result = Dice.Roll(20);
        Assert.InRange(result, 1, 20);
        
        // Test invalid rolls
        Assert.Throws<DiceException>(() => Dice.Roll(0));
        Assert.Throws<DiceException>(() => Dice.Roll(-5));
    }

    [Fact]
    public void TestMultipleRolls()
    {
        Dice.Init(12345);
        
        // Test valid rolls
        var result = Dice.RollMultiple(3, 6);
        Assert.InRange(result, 3, 18);
        
        result = Dice.RollMultiple(1, 20);
        Assert.InRange(result, 1, 20);
        
        // Test invalid rolls
        Assert.Throws<DiceException>(() => Dice.RollMultiple(0, 6));
        Assert.Throws<DiceException>(() => Dice.RollMultiple(3, 0));
    }

    [Fact]
    public void TestIndividualRolls()
    {
        Dice.Init(12345);
        
        // Test valid rolls
        var (sum, individual) = Dice.RollIndividual(3, 6);
        Assert.InRange(sum, 3, 18);
        Assert.Equal(3, individual.Length);
        Assert.All(individual, roll => Assert.InRange(roll, 1, 6));
        Assert.Equal(sum, individual.Sum());
        
        // Test single die
        var (singleSum, singleIndividual) = Dice.RollIndividual(1, 20);
        Assert.InRange(singleSum, 1, 20);
        Assert.Single(singleIndividual);
        Assert.Equal(singleSum, singleIndividual[0]);
    }

    [Fact]
    public void TestNotationRolls()
    {
        Dice.Init(12345);
        
        // Test basic notation
        var result = Dice.RollNotation("1d6");
        Assert.InRange(result, 1, 6);
        
        result = Dice.RollNotation("3d6");
        Assert.InRange(result, 3, 18);
        
        // Test with modifiers
        result = Dice.RollNotation("1d6+5");
        Assert.InRange(result, 6, 11);
        
        result = Dice.RollNotation("1d6-1");
        Assert.InRange(result, 0, 5);
        
        // Test uppercase D
        result = Dice.RollNotation("1D6");
        Assert.InRange(result, 1, 6);
        
        // Test invalid notation
        Assert.Throws<DiceException>(() => Dice.RollNotation("invalid"));
        Assert.Throws<DiceException>(() => Dice.RollNotation(""));
        Assert.Throws<DiceException>(() => Dice.RollNotation(null));
    }
}