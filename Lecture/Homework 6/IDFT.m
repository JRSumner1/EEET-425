function [xOut, Debug] = IDFT(ReX, ImX)
    %
    % IDFT function
    %
    Debug = struct;

    % Number of points in the original time domain
    % If ReX (and ImX) is length N/2+1, then:
    lenFreq = length(ReX);
    numPoints = 2*(lenFreq - 1);  % => N

    % Make sure inputs are row vectors
    ReX = ReX(:).';
    ImX = ImX(:).';

    % Prepare scaled versions
    ReX_bar = zeros(size(ReX));
    ImX_bar = zeros(size(ImX));

    % --- Apply the scale factors per the IDFT approach
    % Typically:
    % - The DC and the Nyquist terms each get a scale of 1/numPoints
    % - All others get a scale of 2/numPoints (for real & imag parts).
    %
    for k = 0:numPoints/2
        if k == 0 || k == numPoints/2
            % DC or Nyquist bin
            ReX_bar(k+1) = ReX(k+1)/numPoints;
            ImX_bar(k+1) = ImX(k+1)/numPoints;  % often 0 for Nyquist if real
        else
            % scale by 2/N
            ReX_bar(k+1) = (2/numPoints)*ReX(k+1);
            ImX_bar(k+1) = (2/numPoints)*ImX(k+1);
        end
    end

    % Store scaled DFT for debugging or checking
    Debug.ScaledReal = ReX_bar;
    Debug.ScaledImag = ImX_bar;

    % Now perform the inverse summation
    xOut = zeros(1, numPoints);

    for i = 0:numPoints-1
        for k = 0:numPoints/2
            xOut(i+1) = xOut(i+1) + ...
                ( ReX_bar(k+1)*cos(2*pi*k*i/numPoints) + ...
                 -ImX_bar(k+1)*sin(2*pi*k*i/numPoints) );
        end
    end
end