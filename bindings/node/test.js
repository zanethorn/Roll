/**
 * Test the Node.js bindings for the Roll dice library
 */

const { Dice, DiceError, init, version, roll, rollMultiple, rollIndividual, rollNotation } = require('./dice.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(`Assertion failed: ${message}`);
    }
}

function testVersion() {
    console.log('Testing version...');
    
    const v = version();
    assert(typeof v === 'string', 'Version should be a string');
    assert(v.length > 0, 'Version should not be empty');
    console.log(`Library version: ${v}`);
    
    console.log('✓ Version test passed\n');
}

function testInitialization() {
    console.log('Testing initialization...');
    
    // Test with seed
    init(12345);
    
    // Test without seed
    init();
    
    // Test with null
    init(null);
    
    console.log('✓ Initialization test passed\n');
}

function testSingleRoll() {
    console.log('Testing single roll...');
    
    init(12345);
    
    // Test valid rolls
    const result1 = roll(6);
    assert(result1 >= 1 && result1 <= 6, `Roll d6 should be 1-6, got ${result1}`);
    
    const result2 = roll(20);
    assert(result2 >= 1 && result2 <= 20, `Roll d20 should be 1-20, got ${result2}`);
    
    console.log(`Rolled d6: ${result1}, d20: ${result2}`);
    console.log('✓ Single roll test passed\n');
}

function testMultipleRolls() {
    console.log('Testing multiple rolls...');
    
    init(12345);
    
    // Test valid rolls
    const result1 = rollMultiple(3, 6);
    assert(result1 >= 3 && result1 <= 18, `Roll 3d6 should be 3-18, got ${result1}`);
    
    const result2 = rollMultiple(1, 20);
    assert(result2 >= 1 && result2 <= 20, `Roll 1d20 should be 1-20, got ${result2}`);
    
    console.log(`Rolled 3d6: ${result1}, 1d20: ${result2}`);
    console.log('✓ Multiple rolls test passed\n');
}

function testIndividualRolls() {
    console.log('Testing individual rolls...');
    
    init(12345);
    
    // Test valid rolls
    const { sum, individual } = rollIndividual(3, 6);
    assert(sum >= 3 && sum <= 18, `Sum should be 3-18, got ${sum}`);
    assert(Array.isArray(individual), 'Individual should be an array');
    assert(individual.length === 3, `Individual should have 3 elements, got ${individual.length}`);
    
    for (const roll of individual) {
        assert(roll >= 1 && roll <= 6, `Each roll should be 1-6, got ${roll}`);
    }
    
    const calculatedSum = individual.reduce((a, b) => a + b, 0);
    assert(sum === calculatedSum, `Sum ${sum} should equal calculated sum ${calculatedSum}`);
    
    console.log(`Rolled 3d6: ${individual.join(', ')} = ${sum}`);
    console.log('✓ Individual rolls test passed\n');
}

function testNotationRolls() {
    console.log('Testing notation rolls...');
    
    init(12345);
    
    // Test basic notation
    const result1 = rollNotation("1d6");
    assert(result1 >= 1 && result1 <= 6, `1d6 should be 1-6, got ${result1}`);
    
    const result2 = rollNotation("3d6");
    assert(result2 >= 3 && result2 <= 18, `3d6 should be 3-18, got ${result2}`);
    
    // Test with modifiers
    const result3 = rollNotation("1d6+5");
    assert(result3 >= 6 && result3 <= 11, `1d6+5 should be 6-11, got ${result3}`);
    
    const result4 = rollNotation("1d6-1");
    assert(result4 >= 0 && result4 <= 5, `1d6-1 should be 0-5, got ${result4}`);
    
    // Test uppercase D
    const result5 = rollNotation("1D6");
    assert(result5 >= 1 && result5 <= 6, `1D6 should be 1-6, got ${result5}`);
    
    console.log(`Notation tests: 1d6=${result1}, 3d6=${result2}, 1d6+5=${result3}, 1d6-1=${result4}, 1D6=${result5}`);
    
    // Test multiple rolls
    const multiResults = rollNotation("2d6", 3);
    assert(Array.isArray(multiResults), 'Multiple rolls should return array');
    assert(multiResults.length === 3, `Should have 3 results, got ${multiResults.length}`);
    
    for (const result of multiResults) {
        assert(result >= 2 && result <= 12, `Each 2d6 should be 2-12, got ${result}`);
    }
    
    console.log(`Multiple 2d6 rolls: ${multiResults.join(', ')}`);
    
    // Test invalid notation
    try {
        rollNotation("invalid");
        assert(false, 'Should have thrown error for invalid notation');
    } catch (error) {
        assert(error instanceof DiceError, 'Should throw DiceError');
    }
    
    console.log('✓ Notation rolls test passed\n');
}

function testClassInterface() {
    console.log('Testing class interface...');
    
    const d = new Dice();
    d.init(12345);
    
    const result1 = d.roll(6);
    assert(result1 >= 1 && result1 <= 6, `Class roll d6 should be 1-6, got ${result1}`);
    
    const result2 = d.rollMultiple(3, 6);
    assert(result2 >= 3 && result2 <= 18, `Class roll 3d6 should be 3-18, got ${result2}`);
    
    const { sum, individual } = d.rollIndividual(3, 6);
    assert(sum === individual.reduce((a, b) => a + b, 0), 'Sum should match individual total');
    
    const result3 = d.rollNotation("2d6+3");
    assert(result3 >= 5 && result3 <= 15, `2d6+3 should be 5-15, got ${result3}`);
    
    console.log(`Class tests: d6=${result1}, 3d6=${result2}, individual=${individual.join(', ')}, 2d6+3=${result3}`);
    console.log('✓ Class interface test passed\n');
}

function runAllTests() {
    console.log('Running Node.js dice library tests...\n');
    
    try {
        testVersion();
        testInitialization();
        testSingleRoll();
        testMultipleRolls();
        testIndividualRolls();
        testNotationRolls();
        testClassInterface();
        
        console.log('🎉 All Node.js tests passed!');
    } catch (error) {
        console.error('❌ Test failed:', error.message);
        process.exit(1);
    }
}

if (require.main === module) {
    runAllTests();
}