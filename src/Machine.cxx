#include "include/Machine.hxx"

Machine::Machine()
{
    std::cout << "Machine created" << std::endl;
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    // reset timers
    dt = 0;
    st = 0;

    // clear memory
    for (uint8_t &i : memory)
    {
        i = 0;
    }
    // clear registers V0-VF
    for (uint8_t &i : V)
    {
        i = 0;
    }
    // clear stack
    for (uint16_t &i : stack)
    {
        i = 0;
    }
    // clear graphics
    for (uint8_t &i : gfx)
    {
        i = 0;
    }
    // clear keyboard
    for (uint8_t &i : key)
    {
        i = 0;
    }

    // load fontset
    for (int i = 0; i < 80; ++i)
    {
        memory[i] = chip8_fontset[i];
    }
}

void Machine::load_rom(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("Could not open file");

    file.seekg(0, std::ifstream::end);
    int size = (int)file.tellg();

    if (size > 4096 - 512)
        throw std::runtime_error("File too large");

    char *buffer = new char[size];
    file.seekg(0, std::ifstream::beg);
    file.read(buffer, size);
    file.close();
    for (int i = 0; i < size; ++i)
    {
        memory[i + 512] = buffer[i];
    }
    delete[] buffer;
}

void Machine::cycle()
{
    draw = false;

    opcode = memory[pc] << 8 | memory[pc + 1];

    // decode opcode
    switch (opcode & 0xF000)
    {

    case 0x0000:
        switch (opcode & 0x00FF)
        {
        case 0x00E0: // CLS | clear screen
            for (uint8_t &i : gfx)
                i = 0;
            pc += 2;
            draw = true;
            break;

        case 0x00EE: // RET | return from subroutine
            --sp;
            pc = stack[sp];
            pc += 2;
            break;
        default:
            std::cout << "Unknown opcode [0x0000]: " << std::hex << opcode << std::endl;
            break;
        }
        break;

    case 0x2000: // CALL addr | call subroutine at nnn
        stack[sp] = pc;
        ++sp;
        // fall through
    case 0x1000: // JP addr | jump to nnn
        pc = opcode & 0x0FFF;
        break;

    case 0x3000: // SE Vx, byte | skip next instruction if Vx == kk
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;

    case 0x4000: // SNE Vx, byte | skip next instruction if Vx != kk
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;

    case 0x5000: // SE Vx, Vy | skip next instruction if Vx == Vy
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;

    case 0x6000: // LD Vx, byte | set Vx = kk
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;

    case 0x7000: // ADD Vx, byte | set Vx = Vx + kk
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;

    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000: // LD Vx, Vy | set Vx = Vy
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0001: // OR Vx, Vy | set Vx = Vx OR Vy
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0002: // AND Vx, Vy | set Vx = Vx AND Vy
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0003: // XOR Vx, Vy | set Vx = Vx XOR Vy
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00f0) >> 4];
            pc += 2;
            break;

        case 0x0004:
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1; // carry
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
            /*
                        case 0x0004: // ADD Vx, Vy | Set Vx = Vx + Vy, set VF = carry
                            V[0xF] =  V[(opcode & 0x0F00) >> 8 ] > (0xFF - V[(opcode & 0x00F0) >> 4]) ? 1 : 0;
                            V[(opcode & 0x0F00 >> 8 )] += V[(opcode & 0x00F0 >> 4)];
                            pc += 2;
                            break;
                            */
        case 0x0005: // SUB Vx, Vy | Set Vx = Vx - Vy, set VF = NOT borrow
            V[0xF] = V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4] ? 1 : 0;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0006: // SHR Vx {, Vy} | Set Vx = Vx SHR 1
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;

        case 0x0007: // SUBN Vx, Vy | Set Vx = Vy - Vx, set VF = NOT borrow
            V[0xF] = V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8] ? 1 : 0;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x000E: // SHL Vx {, Vy} | Set Vx = Vx SHL 1
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;

        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            break;
        }
        break;

    case 0x9000: // SNE Vx, Vy | Skip next instruction if Vx != Vy
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;

    case 0xA000: // set I to nnn
        I = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xB000: // JP V0, addr | Jump to location nnn + V0
        pc = (opcode & 0x0FFF) + V[0x0];
        break;

    case 0xC000: // RND Vx, byte | Set Vx = random byte AND kk
        V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000: // DRW Vx, Vy, nibble | Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
    {
        uint8_t x = V[(opcode & 0x0F00) >> 8];
        uint8_t y = V[(opcode & 0x00F0) >> 4];
        uint8_t height = opcode & 0x000F;
        uint8_t pixel;
        // display is 64x32 pixels
        // max sprite size is 8x15

        V[0xF] = 0;
        for (int yline = 0; yline < height; yline++)
        {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {

                    if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                        V[0xF] = 1;
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }

        draw = true;
        pc += 2;
        break;
    }

    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E: // SKP Vx | Skip next instruction if key with the value of Vx is pressed
            if (key[V[(opcode & 0x0F00) >> 8]])
                pc += 4;
            else
                pc += 2;
            break;

        case 0x00A1: // SKNP Vx | Skip next instruction if key with the value of Vx is not pressed
            if (!key[V[(opcode & 0x0F00) >> 8]])
                pc += 4;
            else
                pc += 2;
            break;

        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            break;
        }
        break;

    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007: // LD Vx, DT | Set Vx = delay timer value
            V[(opcode & 0x0F00) >> 8] = dt;
            pc += 2;
            break;

        case 0x000A: // LD Vx, K | Wait for a key press, store the value of the key in Vx
            for (uint8_t i = 0; i < 16; ++i)
            {
                if (key[i])
                {
                    V[(opcode & 0x0F00) >> 8] = i;
                    pc += 2;
                    goto timers;
                }
            }
            // do not increment pc here,
            // we want to stay in this instruction (halt) until a key is pressed
            break;

        case 0x0015: // LD DT, Vx | Set delay timer = Vx
            dt = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0018: // LD ST, Vx | Set sound timer = Vx
            st = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x001E: // ADD I, Vx | Set I = I + Vx
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0029: // LD F, Vx | Set I = location of sprite for digit Vx
            // characters are stored in order in memory starting from 0x0, five bytes per character
            I = V[(opcode & 0x0F00) >> 8] * 5;
            pc += 2;
            break;

        case 0x0033: // LD B, Vx | Store BCD representation of Vx in memory locations I, I+1, and I+2
            // The interpreter takes the decimal value of Vx,
            // and places the hundreds digit in memory at location in I,
            //  the tens digit at location I+1, and the ones digit at location I+2.
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
            pc += 2;
            break;

        case 0x0055: // LD [I], Vx | Store registers V0 through Vx in memory starting at location I
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            {
                memory[I + i] = V[i];
            }
            pc += 2;
            break;

        case 0x0065: // LD Vx, [I] | Read registers V0 through Vx from memory starting at location I
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
            {
                V[i] = memory[I + i];
            }
            pc += 2;
            break;

        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            break;
        }
        break;

    default:
        std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
    }

timers:
    if (dt > 0)
        --dt;

    if (st > 0)
    {
        if (st == 1)
            std::cout << "SOUND TIMER" << std::endl;
        --st;
    }
}