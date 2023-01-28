void return_demonstration_1() {
    return; // simple return
}

bool return_demonstration_2() {
    return 1; // return with value
}

char return_demonstration_3() {
    return 1 > 0; // return with expression
}

void ifs_demonstration() {

    // sample if without blocks
    if (true)
        return;

    // sample if/else
    if (true)
        return;
    else
        return;

    // if with blocks
    if (true) {
        return;
    }

    // if/else with blocks
    if (true) {
        return;
    } else {
        return;
    }
}

void while_demonstration() {

    // simple while
    while (true)
        return;

    // with blocks
    while (true) {
        return;
    }

    // with break
    while (true) {
        break;
    }

    // with continue
    while (true) {
        continue;
    }

    // all together now
    while (true) {
        if (false) {
            break;
        } else {
            continue;
        }
    }
}
