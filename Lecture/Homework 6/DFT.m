function [ReX, ImX, Debug] = DFT(inputX)
    %
    % Discrete Fourier Transform function
    % inputX is a row (or column) vector of time samples
    % ReX, ImX are row vectors that store the real and imaginary parts
    % of the DFT from k=0 up to k=N/2
    %

    Debug = struct;         % for debugging or returning extra info

    % -- Make sure input is a row vector
    inputX = inputX(:).';   % force row shape

    % -- Check length is even. If odd, pad with one zero.
    numInputPoints = length(inputX);
    if mod(numInputPoints,2) ~= 0
        inputX(end+1) = 0;  % zero-pad
        numInputPoints = numInputPoints + 1;
    end

    % -- Prepare space for ReX and ImX
    %    N/2 + 1 frequency indices
    N = numInputPoints;
    halfN = N/2;
    ReX   = zeros(1, halfN+1);
    ImX   = zeros(1, halfN+1);

    % -- Double "for" loop to accumulate cosines & sines
    for k = 0:halfN
        for i = 0:N-1
            ReX(k+1) = ReX(k+1) + inputX(i+1) * cos((2*pi*k*i)/N);
            ImX(k+1) = ImX(k+1) - inputX(i+1) * sin((2*pi*k*i)/N);
        end
    end

    % (Optionally save any debugging info you want)
    Debug.N = N;
end